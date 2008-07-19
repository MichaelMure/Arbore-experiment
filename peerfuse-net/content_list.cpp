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
 * 
 */

#include "content_list.h"
#include "environment.h"
#include "peers_list.h"

ContentList content_list;

void ContentList::SendRefFile(pf_id to, std::string filename)
{
	uint32_t ref = content_list.GetRef(filename);
	FileContent& f = content_list.GetFile(filename);
	off_t offset;
	off_t size;
	f.GetSharedContent(offset, size);

	Packet packet(NET_REF_FILE, environment.my_id.Get(), to);
	packet.SetArg(NET_REF_FILE_PATH, filename);
	packet.SetArg(NET_REF_FILE_REF, ref);
	packet.SetArg(NET_REF_FILE_OFFSET, (uint64_t)offset);
	packet.SetArg(NET_REF_FILE_SIZE, (uint64_t)size);
	peers_list.SendMsg(to, packet);

}
