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

#ifndef FILEDIST_H
#define FILEDIST_H

#include <vector>

#include "pf_types.h"
#include "pf_file.h"

class Peer;

const size_t NB_PEERS_PER_FILE = 5;

class FileDistribution
{
	std::vector<id_t> id_list;
	FileList resp_files;

	bool _is_responsible(const id_t peer_id, const FileEntry* file, const std::vector<id_t>& id_list) const;
	std::set<Peer*> _get_peers_from_idlist(const FileEntry* f, const std::vector<id_t>& id_list) const;

public:

	FileDistribution();

	std::set<Peer*> GetPeers(const FileEntry* f) const;
	bool IsResponsible(const id_t peer_id, const FileEntry* file) const;

	FileList GetFiles(id_t id) const;

	FileList GetMyRespFiles() const { return resp_files; }

	void UpdateRespFiles();

	/* Flags are defined in cache.h */
	void AddFile(FileEntry* f, unsigned int flags = 0);
	void RemoveFile(FileEntry* f, unsigned int flags = 0);
};
#endif						  /* FILEDIST_H */
