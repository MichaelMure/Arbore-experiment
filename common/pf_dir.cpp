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

#include "pf_dir.h"

DirEntry::DirEntry(std::string name, pf_stat _stat, DirEntry* _parent)
			: FileEntry(name, _stat, _parent)
{
	/* Set the specific directory flag, and remove file flag. */
	pf_stat stat = GetAttr();
	stat.mode |= S_IFDIR;
	stat.mode &= ~S_IFREG;
	SetAttr(stat);
}

DirEntry::~DirEntry()
{
	/* Remove all entries in this directory. */
	for(FileMap::iterator it = files.begin(); it != files.end(); ++it)
		delete it->second;
}

size_t DirEntry::CountExistantFiles() const
{
	size_t i = 0;
	for(FileMap::const_iterator it = files.begin(); it != files.end(); ++it)
		if(it->second->IsRemoved() == false)
			++i;

	return i;
}

void DirEntry::AddFile(FileEntry* file)
{
	FileMap::iterator it;
	if((it = files.find(file->GetName())) != files.end())
	{
		log[W_DEBUG] << "File already exists, replacing it";
		delete it->second;
	}

	files[file->GetName()] = file;
}

void DirEntry::RemFile(FileEntry* file)
{
	/* TODO: Remove-me ? */
}

FileEntry* DirEntry::GetFile(std::string name) const
{
	FileMap::const_iterator ret = files.find(name);
	if(ret == files.end())
		return NULL;

	return ret->second;
}
