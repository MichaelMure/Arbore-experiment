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
 * $Id$
 */

#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "pf_filebase.h"
#include "pf_dir.h"
#include "session_config.h"

FileEntryBase::FileEntryBase(std::string _name, pf_stat _stat, DirEntry* _parent)
			: name(_name), parent(_parent)
{
	SetAttr(_stat);
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

void FileEntryBase::SetAttr(pf_stat new_stat)
{
	// Update attribute
	if(stat.size != new_stat.size)
		tree_cfg.Set(GetFullName() + "#size", (uint32_t)new_stat.size);
	if(stat.meta_mtime != new_stat.meta_mtime)
		tree_cfg.Set(GetFullName() + "#meta", (uint32_t)new_stat.meta_mtime);
	if(stat.pf_mode != new_stat.pf_mode)
		tree_cfg.Set(GetFullName() + "#pfmode", (uint32_t)new_stat.pf_mode);
	stat = new_stat;
}
