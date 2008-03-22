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

#include "pf_file.h"

bool CompFiles::operator() (const FileEntry* f1, const FileEntry* f2) const
{
	if(f1->IsChildOf(f2))
		return false;
	else if(f2->IsChildOf(f1))
		return true;
	else
		return f1 < f2;
}

FileEntry::FileEntry(std::string _name, DirEntry* parent)
			: FileEntryBase(_name, parent),
			path_serial(0u)
{
	for(std::string::iterator c = _name.begin(); c != _name.end(); ++c)
		path_serial += (path_serial << 3) + (unsigned char)*c;
}
