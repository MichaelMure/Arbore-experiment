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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include "cache_base.h"
#include "dir_entry.h"
#include "pf_file.h"
#include "packet.h"

class Cache : public CacheBase
{
	std::vector<std::string> delayed_mkfile_send;
public:

	Cache();
	~Cache();

	virtual void MkFile(std::string path, pf_stat stat, IDList sharers = IDList(), pf_id sender = 0);
	virtual void RmFile(std::string path);
	virtual void SetAttr(std::string path, pf_stat stat, IDList sharers = IDList(), pf_id sender = 0, bool keep_newest = true, bool erase_on_modification = false);
	virtual void RenameFile(std::string path, std::string new_path, pf_id sender = 0);

	virtual void SendMkFile(std::string filename);

	void SendChanges(pf_id peer, time_t last_view);

	virtual Packet CreateMkFilePacket(FileEntry* file);

};

extern Cache cache;
#endif
