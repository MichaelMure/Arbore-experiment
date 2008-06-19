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

#ifndef DIR_ENTRY_H
#define DIR_ENTRY_H

#include <map>
#include "file_entry.h"

typedef std::map<std::string, FileEntry*> FileMap;

class DirEntry : public FileEntry
{
	FileMap files;

public:

	/** Creator of DirEntry.
	 * @param name dir's name (not full path).
	 * @param stat stat informations. If S_IFREG is set in mode, it will changed to S_IFDIR.
	 * @param parent parent directory (NULL if root dir)
	 */
	DirEntry(std::string name, pf_stat stat, DirEntry* parent);
	~DirEntry();

	const FileMap& GetFiles() { return files; }
	size_t GetSize() const { return files.size(); }

	/** Return number of existant files (which are not marked as removed) */
	size_t CountExistantFiles() const;

	/** Add a file in child list
	 * @param file file to add.
	 */
	void AddFile(FileEntry* file);

	/** Remove file from child list.
	 * It will *not destroy object.
	 */
	void RemFile(FileEntry* file);

	/** Get a file from name. */
	FileEntry* GetFile(std::string name) const;
};
#endif						  /* DIR_ENTRY_H */
