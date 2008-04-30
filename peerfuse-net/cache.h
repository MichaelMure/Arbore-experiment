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
#include <map>
#include "cache_base.h"
#include "pf_dir.h"
#include "pf_types.h"
#include "pf_file.h"
#include "packet.h"
#include "filedist.h"

class Cache : public CacheBase
{

	#ifndef PF_SERVER_MODE
	enum incoming_states_t
	{
		GETTING,
		FINISHED,
	};
	std::map<std::string, incoming_states_t> dir_incoming;

	bool IsReadyForList(std::string path);
	#endif					  /* PF_SERVER_MODE */

	/* Because there are functions which return protected data,
	 * but that FileDistribution object is an own attribute, these
	 * functions are private but filedist *can* call them.
	 * It's so my friend.
	 */
	friend class FileDistribution;
	FileDistribution filedist;

	/* This method will explore all arborescence. It can be
	 * slow, so do NOT call this function too frequently.
	 */
	virtual FileList GetAllFiles();

	/* SetAttr of file, use this method when you already have FileEntry* and Peer* objects */
	void _set_attr(FileEntry* file, pf_stat stat, Peer* sender, IDList sharers);

public:

	Cache();
	~Cache();

	void MkFile(std::string path, pf_stat stat, IDList sharers = IDList(), pf_id sender = 0);
	void RmFile(std::string path);
	void SetAttr(std::string path, pf_stat stat, IDList sharers = IDList(), pf_id sender = 0, bool keep_newest = true);
	void RenameFile(std::string path, std::string new_path, pf_id sender = 0);
	void AddSharer(std::string path, pf_id sender);

	/* Send files dir to a peer */
	void SendDirFiles(std::string path, pf_id to);

#ifndef PF_SERVER_MODE
	void SetReadyForList(std::string path);
	virtual void FillReadDir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
#endif

	/* FileDistributino functions... */
	void UpdateRespFiles();

	void RequestFileRefs(std::string filename);
};

extern Cache cache;
#endif
