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

#ifndef PF_SERVER_MODE
#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 26
#endif						  /* FUSE_USE_VERSION */
#ifndef _XOPEN_SOURCE
#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif						  /* linux */
#endif						  /* _XOPEN_SOURCE */

#include <fuse.h>
#endif /* PF_SERVER_MODE */

#include <vector>
#include "mutex.h"
#include "pf_file.h"
#include "packet.h"

class FileEntry;
class DirEntry;

/** This class is interface for the Cache classes.
 *
 * \warning It is really important to NOT return ANY part
 * of this class from functions (FileEntry*, etc)
 */
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

	virtual void ChOwn(std::string path, uid_t uid, gid_t gid) = 0;
	virtual void ChMod(std::string path, mode_t mode) = 0;
	virtual pf_stat GetAttr(std::string path) = 0;
#ifndef PF_SERVER_MODE
	virtual void FillReadDir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) = 0;
#endif

	virtual void MkFile(std::string path, pf_stat stat, pf_id sender = 0) = 0;
	virtual void RmFile(std::string path, pf_id sender = 0) = 0;
	virtual void RenameFile(std::string path, std::string new_path, pf_id sender = 0) = 0;

	virtual Packet CreateMkFilePacket(FileEntry* file) = 0;
	virtual Packet CreateRmFilePacket(FileEntry* file) = 0;
};
#endif						  /* CACHE_BASE_H */
