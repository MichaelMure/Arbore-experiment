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

#ifndef CONTENT_LIST_H
#define CONTENT_LIST_H

#include <map>
#include <string>

#include "util/mutex.h"
#include "util/pf_thread.h"
#include "files/file_content.h"

class ContentList : public Thread, private Mutex, private std::map<std::string, FileContent>
{
	// Map the ref i'll use to SEND file
	std::map<uint32_t, std::string> my_refs;
	std::map<std::string, KeyList> refered_by;

protected:
	void Loop();
	void OnStop();

public:
	ContentList() : Mutex(RECURSIVE_MUTEX) {}
	virtual ~ContentList();

	/** Returns the content of the file we already have in memory
	 * @param path path to the file
	 * @return the file content
	 */
	FileContent& GetFile(std::string path);
	FileContent& GetFile(uint32_t ref);

	/** Remove a file from the list. It's content is not flushed to the disk
	 * @param path path to the file
	 */
	void RemoveFile(std::string path);

	/** When a peer disconnect acknowledge the file-contents that the peer
	 *  stopped sharing the file
	 * @param path path to the file
	 */
	void RemovePeerRefs(Key peer);

	/** Resend to peers the part of the file we are sharing
	 * @param path path to the file
	 */
	void RefreshPeersRef(std::string path);

	uint32_t GetRef(std::string filename);

	/* Track that this peer is using this ref */
	void AddReferer(std::string path, Key referer);
	/* Stop tracking that this peer was using this ref */
	void DelReferer(std::string path, Key referer);
	/* Stop tracking all the ref a peer has */
	void DelReferer(Key);

	/* Send a NET_REF_FILE message to a peer */
	virtual void SendRefFile(Key to, std::string filename);
};

extern ContentList content_list;

#endif						  /* CONTENT_LIST_H */
