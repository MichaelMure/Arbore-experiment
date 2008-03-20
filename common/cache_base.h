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

#ifndef CACHE_BASE_H
#define CACHE_BASE_H

#include <vector>
#include "mutex.h"
#include "pf_file.h"
#include "packet.h"

class Peer;
class FileEntry;
class DirEntry;

class CacheInterface : public Mutex
{

public:

	/* Exceptions */
	class DirNotEmpty : public std::exception {};
	class FileAlreadyExists : public std::exception {};
	class NoSuchFileOrDir : public std::exception {};
	class NoPermission : public std::exception {};

	CacheInterface() : Mutex(RECURSIVE_MUTEX) {}
	virtual ~CacheInterface() {}

	/** Load all tree from an hard drive path.
	 * It will call the Hdd object to load it.
	 *
	 * @param hd_param path on hard drive
	 */
	virtual void Load(std::string hd_path) = 0;

	virtual DirEntry* GetTree() = 0;

	virtual FileEntry* Path2File(std::string path, std::string *filename = NULL) = 0;

	virtual FileEntry* MkFile(std::string path, mode_t mode, Peer* sender = NULL) = 0;
	virtual void RmFile(std::string path, Peer* sender = NULL) = 0;
	virtual void ModFile(std::string path, Peer* sender = NULL) = 0;

	virtual Packet CreateMkFilePacket(FileEntry* file) = 0;
	virtual Packet CreateRmFilePacket(FileEntry* file) = 0;
};

#endif /* CACHE_BASE_H */
