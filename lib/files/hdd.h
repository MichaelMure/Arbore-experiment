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

#ifndef HDD_H
#define HDD_H

#include <exception>
#include <string>

#include <util/mutex.h>
#include <files/dir_entry.h>
#include <files/file_entry.h>

/**
 * This class allow to access files on disk
 * This class is thread safe.
 */
class Hdd : public Mutex
{
	std::string root;

public:
	/**
	 * Specialization of std::exception to indicate writing failure.
	 */
	class HddWriteFailure : public std::exception
	{
		public:
			std::string dir;
			HddWriteFailure (std::string _dir) : dir(_dir) {};
			~HddWriteFailure () throw() {};
	};

	/**
	 * Specialization of std::exception to indicate reading failure.
	 */
	class HddAccessFailure : public std::exception
	{
		public:
			std::string dir;
			HddAccessFailure (std::string _dir) : dir(_dir) {};
			~HddAccessFailure () throw() {};
	};

	Hdd();
	~Hdd();

	/** Create a tree from an hard drive path.
	 * @param d pointer to the root node of tree.
	 * @param path path to the tree on hard drive.
	 */
	void BuildTree(DirEntry* d, std::string path);

	/** Create on disk a file
	 *
	 * If the parent directory does not exist, it will be created.
	 *
	 * @param f pointer to the FileEntry to create.
	 */
	void MkFile(FileEntry* f);


	void UpdateFile(FileEntry* f) { /* TODO: Not Implemented Yet */ }

	/** Remove a file from the hard drive.
	 *
	 * @param f pointer to the FileEntry to delete.
	 */
	void RmFile(FileEntry* f);

	/** Return the file descriptor corresponding of a path
	 *
	 * If the file can't be read, -1 is returned.
	 * @param path the path of the file
	 * @return the file descriptor
	 */
	int GetFd(std::string path);
};

/** Singleton */
extern Hdd hdd;
#endif
