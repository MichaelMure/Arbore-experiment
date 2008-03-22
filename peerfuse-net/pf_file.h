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

#ifndef PF_FILE_H
#define PF_FILE_H

#include <set>

#include "pf_filebase.h"

class FileEntry;

struct CompFiles
{
	bool operator() (const FileEntry* f1, const FileEntry* f2) const;
};

typedef std::set<FileEntry*, CompFiles> FileList;

class FileEntry : public FileEntryBase
{
	unsigned int path_serial;

public:

	FileEntry(std::string _name, DirEntry* parent);

	/* TODO: calculate this */
	unsigned int GetPathSerial() const { return path_serial; }
};
#endif						  /* PFNET_FILE_H */
