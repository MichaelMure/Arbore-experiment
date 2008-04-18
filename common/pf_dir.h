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

#ifndef PF_DIR_H
#define PF_DIR_H

#include <map>
#include "pf_file.h"

typedef std::map<std::string, FileEntry*> FileMap;

class DirEntry : public FileEntry
{
	FileMap files;

public:

	DirEntry(std::string name, pf_stat stat, DirEntry* parent);
	~DirEntry();

	const FileMap& GetFiles() { return files; }
	size_t GetSize() const { return files.size(); }

	void AddFile(FileEntry* file);
	void RemFile(FileEntry* file);	  /**< will *not* destroy object */

	FileEntry* GetFile(std::string name) const;
};
#endif						  /* PF_DIR_H */
