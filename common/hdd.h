/*
 * Copyright(C) 2008 Laurent Defert
 *                   Romain Bignon <romain AT vaginus.org>
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

#ifndef HDD_H
#define HDD_H
#include <string>
#include <exception>
#include "pf_file.h"
#include "pf_dir.h"
#include "mutex.h"

class Hdd : public Mutex
{
	std::string root;

public:
	class HddWriteFailure : public std::exception
	{
		public:
			std::string dir;
			HddWriteFailure (std::string _dir) : dir(_dir) {};
			~HddWriteFailure () throw() {};
	};
	class HddAccessFailure : public std::exception
	{
		public:
			std::string dir;
			HddAccessFailure (std::string _dir) : dir(_dir) {};
			~HddAccessFailure () throw() {};
	};

	Hdd();
	~Hdd();

	/* Create a tree from an hard drive path.
	 * @param d pointer to the root node of tree.
	 * @param path path to the tree on hard drive.
	 */
	void BuildTree(DirEntry* d, std::string path);

	void MkFile(FileEntry* f);
	void UpdateFile(FileEntry* f) { /* TODO: Not Implemented Yet */ }
	void RmFile(FileEntry* f);

	int GetFd(std::string path);
};

extern Hdd hdd;
#endif
