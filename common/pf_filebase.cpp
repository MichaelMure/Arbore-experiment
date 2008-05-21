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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "pf_filebase.h"
#include "pf_dir.h"
#include "session_config.h"

FileEntryBase::FileEntryBase(std::string _name, pf_stat _stat, DirEntry* _parent)
			: name(_name), parent(_parent), stat(_stat)
{
}

FileEntryBase::~FileEntryBase()
{

}

std::string FileEntryBase::GetFullName() const
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

bool FileEntryBase::IsChildOf(const FileEntryBase* f) const
{
	DirEntry* p = parent;

	while(p && p != f) p = p->parent;

	return (p);
}

void FileEntryBase::SetAttr(pf_stat new_stat, bool force)
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

void FileEntryBase::SetSharers(IDList idl)
{
	sharers = idl;
	#ifdef PF_NET
	std::string list;
	for(IDList::iterator it = sharers.begin(); it != sharers.end(); ++it)
	{
		if(list.empty() == false) list += ",";
		list += TypToStr(*it);
	}
	tree_cfg.Set(GetFullName() + "#sharers", list);
	#endif
}

void FileEntryBase::LoadAttr()
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
}
