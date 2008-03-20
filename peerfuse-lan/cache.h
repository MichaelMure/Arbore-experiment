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

class Cache : public CacheInterface
{
	DirEntry tree;
	Hdd hdd;

public:

	Cache();
	~Cache();

	/** Load all tree from an hard drive path.
	 * It will call the Hdd object to load it.
	 *
	 * @param hd_param path on hard drive
	 */
	virtual void Load(std::string hd_path);

	virtual DirEntry* GetTree() { return &tree; }

	virtual FileEntry* Path2File(std::string path, std::string *filename = NULL);

	virtual FileEntry* MkFile(std::string path, mode_t mode, Peer* sender = 0);
	virtual void RmFile(std::string path, Peer* sender = 0);
	virtual void ModFile(std::string path, Peer* sender = 0);

	virtual void SendChanges(Peer* p, time_t last_view);

	virtual Packet CreateMkFilePacket(FileEntry* file);
	virtual Packet CreateRmFilePacket(FileEntry* file);

	std::vector<FileEntry*> GetModifiedEntries(time_t last_conn);
};

extern Cache cache;
#endif
