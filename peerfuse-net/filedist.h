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

#ifndef FILEDIST_H
#define FILEDIST_H

#include <vector>

#include "pf_types.h"
#include "pf_file.h"
#include "packet.h"

class Peer;

const size_t NB_PEERS_PER_FILE = 2;

class FileDistribution
{
	std::vector<pf_id> id_list;
	FileList resp_files;

	/** This is an internal function to know if an ID is responsible of a file.
	 * @param peer_id id of peer
	 * @param file FileEntry*
	 * @param id_list list of ids on network.
	 * @return true if peer_id is responsible of file.
	 */
	bool _is_responsible(const pf_id peer_id, const FileEntry* file, const std::vector<pf_id>& id_list) const;

	/** This is an internal function to get responsible peers of a file.
	 * @param f file
	 * @param id_list list of ids on network.
	 * @return set of Peer*
	 */
	std::set<Peer*> _get_resp_peers_from_idlist(const FileEntry* f, const std::vector<pf_id>& id_list) const;

public:

	FileDistribution();

	/** Get responsibles peers of a file.
	 * It calls the internal _get_peers_from_idlist() function with
	 * this->id_list as parameter.
	 *
	 * @param f file
	 * @return set of Peer*
	 */
	std::set<Peer*> GetRespPeers(const FileEntry* f) const;

	/** Is this peer responsible of that file?
	 * It calls the internal _is_responsible() method with this->id_list
	 * as parameter.
	 *
	 * @param peer_id id of peer
	 * @param file FileEntry*
	 * @return true if peer_id is responsible of file.
	 */
	bool IsResponsible(const pf_id peer_id, const FileEntry* file) const;

	/** Get files whose peer is responsible. */
	FileList GetFiles(pf_id id) const;

	FileList GetMyRespFiles() const { return resp_files; }

	/** When there are new peers on network, call this method. */
	void UpdateRespFiles();

	/** Send a NET_WANT_FILE_REF to all sharers of a file. */
	void RequestFileRefs(const FileEntry*);

	/* Flags are defined in cache.h */
	void AddFile(FileEntry* f, Peer* sender);

	void AddSharer(FileEntry* f, pf_id);

	void SendDirFiles(DirEntry* dir, Peer* to);

	/** Create a packet that create a file.
	 * @param file file to send
	 * @return packet created
	 */
	Packet CreateMkFilePacket(FileEntry* file);
};
#endif						  /* FILEDIST_H */
