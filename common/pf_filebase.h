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

#ifndef P2PFS_FILE_H
#define P2PFS_FILE_H

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "file_perms.h"

class DirEntry;

class pf_stat : public FilePermissions
{
public:
	off_t size;
	time_t atime;
	time_t mtime;
	time_t ctime;
};

class FileEntryBase
{
	const std::string name;
	DirEntry* parent;

public:

	pf_stat stat;

	FileEntryBase(std::string name, DirEntry* _parent);
	virtual ~FileEntryBase();

	DirEntry* GetParent() const { return parent; }
	std::string GetName() const { return name; }
	std::string GetFullName();
};

#endif /* P2PFS_FILE_H */
