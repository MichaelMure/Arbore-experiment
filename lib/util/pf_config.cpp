/* LibConfig.cpp - This is a library to make a configuration
 *
 * Copyright (C) 2006 Romain Bignon  <progs@headfucking.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 *
 */

#include <fstream>
#include <iostream>
#include "pf_config.h"

MyConfig conf;

/* Some usefull macro */
#undef FOR
#define FOR(T, v, x) \
	T (x); \
	for(T::iterator x##it = (v).begin(); x##it != (v).end() && (x = x##it->second); ++x##it)
#undef FORit
#define FORit(T, v, x) \
	for(T::iterator (x) = (v).begin(); (x) != (v).end(); ++(x))
#define FORmm(T, v, label, x) \
	T::iterator x##lb = v.lower_bound(label); \
	T::iterator x##ub = v.upper_bound(label); \
	for(T::iterator x = x##lb; x != x##ub; ++x)

std::string stringtok(std::string &in, const char * const delimiters = " \t\n");

#define Error(x) do { std::cerr << path_ << ":" << line_count_ << ": " << x << std::endl ; error = true; } while(0)

/********************************************************************************************
 *                                Config                                                    *
 ********************************************************************************************/

MyConfig::MyConfig(std::string path)
			: path_(path), loaded_(false)
{

}

MyConfig::MyConfig()
			: loaded_(false)
{

}

MyConfig::~MyConfig()
{
	FORit(SectionMap, sections_, it)
		delete it->second;
}

bool MyConfig::Load(std::string path)
{
	if(!path.empty())
		this->path_ = path;
	else
		throw error_exc("Filename is empty");

	std::ifstream fp(path_.c_str());

	if(!fp)
	{
		std::cerr << path_ + ": File not found" << std::endl;
		return false;
	}

	std::string ligne;
	line_count_ = 0;
	bool error = false;
	ConfigSection* section = 0;
	while(std::getline(fp, ligne))
	{
		++line_count_;

		const char* ptr = ligne.c_str();
		while(ptr && *ptr && (*ptr == ' ' || *ptr == '\t'))
			++ptr;

		ligne = ptr;

		if(ligne.empty() || ligne[0] == '#' || ligne[0] == '\r' || ligne[0] == '\n')
			continue;

		if(ligne.find('=') != std::string::npos)
		{
			if(!section)
			{
				Error("We aren't in a section !");
				continue;
			}
			std::string label = stringtok(ligne, " ");
			ConfigItem* item = section->GetItem(label);
			if(!item)
			{
				Error("Unknown item '" << label << "'");
				continue;
			}
			if(ligne.empty())
			{
				Error("There isn't any value");
				continue;
			}
			std::string value;
			ptr = ligne.c_str();
			while(ptr && *ptr && (*ptr == ' ' || *ptr == '=' || *ptr == '\t'))
				++ptr;
			if(!ptr || !*ptr)
			{
				Error("There isn't any value");
				continue;
			}

			value = ptr;

			if(section->NameItem() == item)
			{
				ConfigSection* s = 0;
				if(section->Parent())
					s = section->Parent()->GetSection(section->Label(), value);
				else
					s = GetSection(section->Label(), value);

				if(s)
				{
					Error("There is already a section named '" << value << "' ('" << s->Label() << "')");
					continue;
				}
				section->SetName(value);
			}

			item->SetFound();

			if(!item->SetValue(value))
				Error("'" << value << "' is an incorrect value for '" << item->Label() << "' item (type of value is " <<
					item->ValueType() << ")");
			else if(item->CallBack())
				item->CallBack() (item);
		}
		else if(!ligne.empty() && ligne[0] == '}')
		{
			if(!section)
			{
				Error("a '}' out of a section !?");
				continue;
			}
			if(section->IsCopy() && section->Name().empty())
				Error("Section '" << section->Label() << "' hasn't a name");

			/* This function is usefull to set default values for items,
			 * and to detect if an item is missing in this section.
			 * This function will send errors.
			 */
			if(section->FindEmpty())
				error = true;
			else if(section->EndOfTab())
				section->EndOfTab() (section);
			section = section->Parent();
		}
		else
		{
			std::string tab = stringtok(ligne, " ");
			if(!section)
				section = GetSection(tab);
			else
				section = section->GetSection(tab);

			if(!section)
			{
				Error("Unknown section '" << tab << "'");
				continue;
			}
			section->SetFound();
			if(section->IsMultiple())
			{
				section = new ConfigSection(*section);
				section->SetCopy();
				if(section->Parent())
					section->Parent()->AddSection(section);
				else
					AddSection(section);
			}
		}
	}

	if(section)
		Error("in «" + section->Label() + (section->Name().empty() ? "" : ("(" + section->Name() + ")")) +
		"»: '}' not found to close section !");

	if(FindEmpty())				  // Find empty sections
		error = true;

	if(!error)
		loaded_ = true;
	return !error;
}

ConfigSection* MyConfig::GetSection(std::string label)
{
	FORmm(SectionMap, sections_, label, it)
		if(it->second->IsCopy() == false)
		return it->second;
	return 0;
}

ConfigSection* MyConfig::GetSection(std::string label, std::string name)
{
	FORmm(SectionMap, sections_, label, it)
		if(it->second->Label() == label && it->second->IsMultiple() && it->second->Name() == name)
		return it->second;

	return 0;
}

std::vector<ConfigSection*> MyConfig::GetSectionClones(std::string label)
{
	std::vector<ConfigSection*> s;
	FORmm(SectionMap, sections_, label, it)
		if(it->second->Label() == label && it->second->IsMultiple() && it->second->IsCopy())
		s.push_back(it->second);

	return s;
}

ConfigSection* MyConfig::AddSection(ConfigSection* section)
{
	sections_.insert(std::make_pair(section->Label(), section));
	return section;
}

ConfigSection* MyConfig::AddSection(std::string label, std::string description, bool multiple)
{
	if(loaded_)
		throw error_exc("Configuration is already loaded !");

	FORit(SectionMap, sections_, it)
		if(it->second->Label() == label)
			throw error_exc("Section " + label + " has a name already used");

	return AddSection(new ConfigSection(label, description, multiple, this, 0));
}

bool MyConfig::FindEmpty()
{
	std::string begin = "in global porty: ";
	bool error = false;			  // Error() macro change this value

	FORit(SectionMap, sections_, it)
		if(!it->second->Found())
		{
			ConfigSection* s = it->second;
			if(s->IsMultiple() == false)
				Error(begin << "missing section '" << s->Label() << "' (" << s->Description() << ")");
		}

	return error;
}

/********************************************************************************************
 *                                ConfigSection                                             *
 ********************************************************************************************/

ConfigSection::ConfigSection(std::string label, std::string description, bool multiple,
			MyConfig* config, ConfigSection* parent)
			: label_(label), description_(description), multiple_(multiple), config_(config), parent_(parent), copy_(false),
			name_item_(0), found_(false), end_func_(0)
{
	items_.clear();
}

ConfigSection::ConfigSection(ConfigSection& cs)
			: label_(cs.label_), description_(cs.description_), multiple_(cs.multiple_), config_(cs.config_), parent_(cs.parent_),
			copy_(cs.copy_), name_item_(0), found_(cs.found_), end_func_(cs.end_func_)
{
	FORit(SectionMap, cs.sections_, it)
		sections_.insert(std::make_pair(it->second->label_, new ConfigSection(*it->second)));

	FORit(ItemMap, cs.items_, it)
	{
		ConfigItem* item = it->second->Clone();
		items_[it->second->Label()] = item;
		if(cs.name_item_ == it->second)
			name_item_ = item;
	}
}

ConfigSection::~ConfigSection()
{
	FORit(SectionMap, sections_, it)
		delete it->second;

	FORit(ItemMap, items_, it)
		delete it->second;
}

ConfigSection* ConfigSection::GetSection(std::string label)
{
	FORmm(SectionMap, sections_, label, it)
		if(it->second->IsCopy() == false)
			return it->second;
	return 0;
}

ConfigSection* ConfigSection::GetSection(std::string label, std::string name)
{
	FORmm(SectionMap, sections_, label, it)
		if(it->second->Label() == label && it->second->IsMultiple() && it->second->Name() == name)
			return it->second;

	return 0;
}

std::vector<ConfigSection*> ConfigSection::GetSectionClones(std::string label)
{
	std::vector<ConfigSection*> s;
	FORmm(SectionMap, sections_, label, it)
		if(it->second->Label() == label && it->second->IsMultiple() && it->second->IsCopy())
			s.push_back(it->second);

	return s;
}

bool ConfigSection::FindEmpty()
{
	std::string begin = "in «" + Label() + (Name().empty() ? "" : ("(" + Name() + ")")) + "»: ";
	bool error = false;			  // Error() macro change this value
	int line_count_ = config_->NbLines();		  /* Récuperation des informations de MyConfig */
	std::string path_ = config_->Path();	  /* pour pouvoir utiliser la macro Error()    */

	FORit(ItemMap, items_, it)
		if(!it->second->Found())
		{
			ConfigItem* item = it->second;
			if(item->DefValue().empty() || item->SetValue(item->DefValue()) == false)
				Error(begin << "missing item '" << item->Label() << "' (" << item->Description() << ")");
		}

	FORit(SectionMap, sections_, it)
		if(!it->second->Found() && it->second->IsMultiple() == false)
		{
			ConfigSection* s = it->second;
			Error(begin << "missing section '" << s->Label() << "' (" << s->Description() << ")");
		}

	return error;
}

ConfigSection* ConfigSection::AddSection(ConfigSection* section)
{
	sections_.insert(std::make_pair(section->Label(), section));
	return section;
}

ConfigSection* ConfigSection::AddSection(std::string label, std::string description, bool multiple)
{
	FORit(SectionMap, sections_, it)
		if(it->second->Label() == label)
			throw MyConfig::error_exc("Section " + label + " has a name already used in section \"" + Label() + "\"");

	return AddSection(new ConfigSection(label, description, multiple, config_, this));
}

ConfigItem* ConfigSection::GetItem(std::string label)
{
	ItemMap::iterator it = items_.find(label);
	if(it == items_.end())
		return 0;
	else
		return it->second;
}

void ConfigSection::AddItem(ConfigItem* item, bool is_name)
{
	try
	{
		if(!item)
			throw MyConfig::error_exc("You have to give an item !!");

		if(items_[item->Label()] != 0)
			throw MyConfig::error_exc("There is already an item in \"" + Label() + "\" section named \"" + item->Label() + "\"");

		if(is_name && name_item_)
			throw MyConfig::error_exc("I want to add an 'is_name' item, but I have already one !");

		item->config_ = config_;
		item->parent_ = this;

		items_[item->Label()] = item;
		if(is_name)
			name_item_ = item;
	}
	catch(...)
	{
		delete item;
		throw;
	}
}

/********************************************************************************************
 *                                ConfigItem                                                *
 ********************************************************************************************/

ConfigItem* ConfigItem_int::Clone() const
{
  return new ConfigItem_int(Label(), Description(), min_, max_, DefValue(), CallBack(), GetConfig(), Parent());
}

bool ConfigItem_int::SetValue(std::string s)
{
  for(std::string::const_iterator it = s.begin(); it != s.end(); ++it)
    if(!isdigit(*it)) return false;
  std::istringstream(s) >> value_;
  return (value_ >= min_ && value_ <= max_);
}

std::string ConfigItem_int::ValueType() const
{
  if(min_ == INT_MIN)
		return "integer";
  else
  {
		std::ostringstream off;
		std::string in, ax;
		off << min_;
		in = off.str();
		off << max_;
		ax = off.str();
		return "integer (between " + in + " and " + ax + ")";
  }
}

ConfigItem* ConfigItem_bool::Clone() const
{
  return new ConfigItem_bool(Label(), Description(), DefValue(), CallBack(), GetConfig(), Parent());
}

bool ConfigItem_bool::SetValue(std::string s)
{
  if(s == "true" || s == "on" || s == "yes")
		value_ = true;
  else if(s == "false" || s == "off" || s == "no")
		value_ = false;
  else
    return false;
  return true;
}

std::string ConfigItem_bool::ValueType() const { return "boolean ('true' or 'false')"; }



