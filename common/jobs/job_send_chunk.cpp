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

#include "job_send_chunk.h"
#include "content_list.h"
#include "file_content.h"
#include "peers_list.h"
#include "log.h"

bool JobSendChunk::Start()
{
	FileContent& f = content_list.GetFile(ref);
	FileChunkDesc requested_chunk(offset, size);

	if(f.FileContentHaveChunk(requested_chunk)
						  /* TODO: make OnDiskHaveChunk non-blockant and repeat the job */
		|| f.OnDiskHaveChunk(requested_chunk))
	{
		log[W_DEBUG] << "Sending chunk";
		FileChunk chunk = f.GetChunk(requested_chunk);
		if(chunk.GetData() == NULL)
		{
			log[W_ERR] << "Trying to send an empty chunk..";
			return false;
		}
		peers_list.SendChunk(ref, sendto, chunk);
		return false;
	}
	else
		log[W_DEBUG] << "I don't have this chunk ??";

	return false;
}
