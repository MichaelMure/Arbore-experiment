/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "dir_entry.h"
#include <util/session_config.h>

bool CompFiles::operator() (const FileEntry* f1, const FileEntry* f2) const
{
	if(f1->IsChildOf(f2))
		return false;
	else if(f2->IsChildOf(f1))
		return true;
	else
		return f1 < f2;
}

FileEntry::FileEntry(std::string _name, pf_stat _stat, DirEntry* _parent)
	: name(_name),
	  parent(_parent),
	  stat(_stat)
{
	key.MakeHash(name);
}

FileEntry::~FileEntry()
{

}

std::string FileEntry::GetFullName() const
{
	DirEntry* p = parent;
	std::string path = "";

	while(p)
	{
		path = p->GetName() + "/" + path;
		p = p->parent;
	}

	path = path + name;
	return path;
}

bool FileEntry::IsChildOf(const FileEntry* f) const
{
	DirEntry* p = parent;

	while(p && p != f) p = p->parent;

	return (p);
}

void FileEntry::SetAttr(pf_stat new_stat, bool force)
{
	// Update attribute
	if(stat.size != new_stat.size || force)
		tree_cfg.Set(GetFullName() + "#size", (uint32_t)new_stat.size);
	if(stat.meta_mtime != new_stat.meta_mtime || force)
		tree_cfg.Set(GetFullName() + "#meta", (uint32_t)new_stat.meta_mtime);
	if(stat.pf_mode != new_stat.pf_mode || force)
		tree_cfg.Set(GetFullName() + "#pfmode", (uint32_t)new_stat.pf_mode);
	if(stat.uid != new_stat.uid || force)
		tree_cfg.Set(GetFullName() + "#uid", (uint32_t)new_stat.uid);
	if(stat.gid != new_stat.gid || force)
		tree_cfg.Set(GetFullName() + "#gid", (uint32_t)new_stat.gid);
	if(stat.mode != new_stat.mode || force)
		tree_cfg.Set(GetFullName() + "#mode", (uint32_t)new_stat.mode);
	stat = new_stat;
}

void FileEntry::SetSharers(KeyList idl)
{
	sharers = idl;
	std::string list;
	for(KeyList::iterator it = sharers.begin(); it != sharers.end(); ++it)
	{
		if(list.empty() == false) list += ",";
		list += TypToStr(*it);
	}
	tree_cfg.Set(GetFullName() + "#sharers", list);
}

void FileEntry::LoadAttr()
{
	uint32_t cfg_val = 0;
	if(tree_cfg.Get(GetFullName() + "#meta", cfg_val))
		stat.meta_mtime = (time_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#size", cfg_val))
		stat.size = (size_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#pfmode", cfg_val))
		stat.pf_mode = (uint32_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#uid", cfg_val))
		stat.uid = (uint32_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#gid", cfg_val))
		stat.gid = (uint32_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#mode", cfg_val))
		stat.mode = (uint32_t)cfg_val;
	std::string cfg_val_s;
	if(tree_cfg.Get(GetFullName() + "#sharers", cfg_val_s))
	{
		KeyList keylist;
		std::string key;
		while((key = stringtok(cfg_val_s, ",")).empty() == false)
			keylist.insert(Key(key));
		SetSharers(keylist);
	}

}
