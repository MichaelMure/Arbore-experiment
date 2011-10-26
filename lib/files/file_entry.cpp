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

pf_stat::pf_stat()
	: size(0),
	atime(0),
	mtime(0),
	ctime(0),
	meta_mtime(0)
{}

bool pf_stat::operator==(const pf_stat& st)
{
	return (size == st.size &&
		atime == st.atime &&
		mtime == st.mtime &&
		ctime == st.ctime &&
		meta_mtime == st.meta_mtime &&
		pf_mode == st.pf_mode &&
		mode == st.mode &&
		uid == st.uid &&
		gid == st.gid);
}

bool CompFiles::operator() (const FileEntry* f1, const FileEntry* f2) const
{
	if(f1->IsChildOf(f2))
		return false;
	else if(f2->IsChildOf(f1))
		return true;
	else
		return f1 < f2;
}

FileEntry::FileEntry(std::string name, pf_stat stat, DirEntry* parent)
	: stat_(stat),
		name_(name),
	  parent_(parent)
{
	key_.MakeHash(name_);
}

DirEntry* FileEntry::GetParent() const
{
	return parent_;
}

std::string FileEntry::GetName() const
{
	return name_;
}

std::string FileEntry::GetFullName() const
{
	DirEntry* p = parent_;
	std::string path = "";

	while(p)
	{
		path = p->GetName() + "/" + path;
		p = p->parent_;
	}

	path = path + name_;
	return path;
}

Key FileEntry::getPathSerial() const
{
	return key_;
}

bool FileEntry::IsChildOf(const FileEntry* f) const
{
	DirEntry* p = parent_;

	while(p && p != f) p = p->parent_;

	return (p);
}

KeyList FileEntry::GetSharers() const
{
	return sharers_;
}

void FileEntry::SetSharers(KeyList idl)
{
	sharers_ = idl;
	std::string list;
	for(KeyList::iterator it = sharers_.begin(); it != sharers_.end(); ++it)
	{
		if(list.empty() == false) list += ",";
		list += TypToStr(*it);
	}
	tree_cfg.Set(GetFullName() + "#sharers", list);
}

void FileEntry::AddSharer(Key id)
{
	sharers_.insert(id);
	SetSharers(sharers_); /* Non-awesome */
}

pf_stat FileEntry::GetAttr() const
{
	return stat_;
}

void FileEntry::SetAttr(pf_stat new_stat, bool force)
{
	// Update attribute
	if(stat_.size != new_stat.size || force)
		tree_cfg.Set(GetFullName() + "#size", (uint32_t)new_stat.size);
	if(stat_.meta_mtime != new_stat.meta_mtime || force)
		tree_cfg.Set(GetFullName() + "#meta", (uint32_t)new_stat.meta_mtime);
	if(stat_.pf_mode != new_stat.pf_mode || force)
		tree_cfg.Set(GetFullName() + "#pfmode", (uint32_t)new_stat.pf_mode);
	if(stat_.uid != new_stat.uid || force)
		tree_cfg.Set(GetFullName() + "#uid", (uint32_t)new_stat.uid);
	if(stat_.gid != new_stat.gid || force)
		tree_cfg.Set(GetFullName() + "#gid", (uint32_t)new_stat.gid);
	if(stat_.mode != new_stat.mode || force)
		tree_cfg.Set(GetFullName() + "#mode", (uint32_t)new_stat.mode);
	stat_ = new_stat;
}

void FileEntry::LoadAttr()
{
	uint32_t cfg_val = 0;
	if(tree_cfg.Get(GetFullName() + "#meta", cfg_val))
		stat_.meta_mtime = (time_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#size", cfg_val))
		stat_.size = (size_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#pfmode", cfg_val))
		stat_.pf_mode = (uint32_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#uid", cfg_val))
		stat_.uid = (uint32_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#gid", cfg_val))
		stat_.gid = (uint32_t)cfg_val;
	if(tree_cfg.Get(GetFullName() + "#mode", cfg_val))
		stat_.mode = (uint32_t)cfg_val;
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


bool FileEntry::IsRemoved() const
{
	return stat_.pf_mode & pf_stat::S_PF_REMOVED;
}

void FileEntry::SetRemoved()
{
	stat_.pf_mode |= pf_stat::S_PF_REMOVED;
}

void FileEntry::ClearRemoved()
{
	stat_.pf_mode |= pf_stat::S_PF_REMOVED;
}
