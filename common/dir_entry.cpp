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
 * $Id$
 */

#include "dir_entry.h"

DirEntry::DirEntry(std::string name, pf_stat _stat, DirEntry* _parent)
			: FileEntry(name, _stat, _parent)
{
	stat.mode &= ~S_IFREG;
	stat.mode |= S_IFDIR;
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
		pf_log[W_DEBUG] << "File already exists, replacing it";
		delete it->second;
	}

	files[file->GetName()] = file;
}

void DirEntry::RemFile(FileEntry* file)
{
	FileMap::iterator it = files.find(file->GetName());
	if(it != files.end())
		files.erase(it);
}

FileEntry* DirEntry::GetFile(std::string name) const
{
	FileMap::const_iterator ret = files.find(name);
	if(ret == files.end())
		return NULL;

	return ret->second;
}
