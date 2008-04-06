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

#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include "cache_base.h"
#include "pf_dir.h"
#include "pf_file.h"
#include "packet.h"
#include "hdd.h"
#include "filedist.h"

class Cache : public CacheInterface
{
	DirEntry tree;
	Hdd hdd;
	FileDistribution filedist;
	std::vector<FileEntry*> files;

protected:

	/* Because there are functions which return protected data,
	 * but that FileDistribution object is an own attribute, these
	 * functions are private but filedist *can* call them.
	 * It's so my friend.
	 */
	friend class FileDistribution;

	virtual DirEntry* GetTree() { return &tree; }

	/* This method will explore all arborescence. It can be
	 * slow, so do NOT call this function too frequently.
	 */
	virtual FileList GetAllFiles();

	FileEntry* Path2File(std::string path, std::string *filename = NULL);

public:

	Cache();
	~Cache();

	/** Load all tree from an hard drive path.
	 * It will call the Hdd object to load it.
	 *
	 * @param hd_param path on hard drive
	 */
	virtual void Load(std::string hd_path);

	virtual void ChOwn(std::string path, uid_t uid, gid_t gid);
	virtual void ChMod(std::string path, mode_t mode);
	virtual pf_stat GetAttr(std::string path);
	virtual void FillReadDir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

	void MkFile(std::string path, pf_stat stat, pf_id sender = 0);
	void RmFile(std::string path, Peer* sender = NULL);
	void RenameFile(std::string path, std::string new_path, Peer* sender = NULL);

	Packet CreateMkFilePacket(FileEntry* file);
	Packet CreateRmFilePacket(FileEntry* file);

	/* FileDistributino functions... */
	void UpdateRespFiles();
};

extern Cache cache;
#endif
