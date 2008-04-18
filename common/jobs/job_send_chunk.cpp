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

bool JobSendChunk::Start()
{
	FileContent& f = content_list.GetFile(ref);
	FileChunk requested_chunk(NULL, offset, size);

	if(f.FileContentHaveChunk(offset, size)
	|| f.OnDiskHaveChunk(requested_chunk)) /* TODO: make OnDiskHaveChunk non-blockant and repeat the job */
	{
		FileChunk chunk = f.GetChunk(offset, size);
		peers_list.SendChunk(f.GetFilename(), sendto, chunk);
		return false;
	}

	return false;
}
