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

#include "content_list.h"
#include "pf_types.h"
#include "packet.h"
#include "peers_list.h"

ContentList content_list;

bool ContentList::HaveFile(std::string filename)
{
	/* No need to lock */
	FileContent& f = GetFile(filename);
	return f.HaveAnyChunk();
}

void ContentList::SetSharer(std::string filename, pf_id sharer)
{
	// Don't move-me to job_set_sharer.cpp -> we'll be needed to handle redondancy
	/* No need to lock */
	FileContent& f = GetFile(filename);
	if(f.WantsChunks())
	{
		/* Ask for a ref */
		Packet p(NET_WANT_REF_FILE);
		p.SetArg(NET_WANT_REF_FILE_PATH, filename);
		peers_list.SendMsg(sharer, p);
	}
}
