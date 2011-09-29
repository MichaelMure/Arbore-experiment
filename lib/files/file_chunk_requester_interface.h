/*
 * Copyright(C) 2008 Romain Bignon
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

#ifndef FILE_CHUNK_REQUESTER_INTERFACE_H
#define FINE_CHUNK_REQUESTER_INTERFACE_H

#include <string>

class FileChunkRequesterInterface
{
public:

	virtual bool IsConnected() = 0;

	virtual bool RequestFileRefs(std::string filename) = 0;

	virtual void RequestChunk(std::string filename, Key key,
};

#endif /* FILE_CHUNK_REQUESTER_INTERFACE_H */
