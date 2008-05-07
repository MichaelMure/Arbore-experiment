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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>				  // fcntl
#include <fcntl.h>				  // fcntl
#include "connection.h"
#include "log.h"

Connection::Connection(int _fd) : fd(_fd),
			read_buf(NULL),
			read_buf_size(0),
			write_buf(NULL),
			write_buf_size(0)
{
	// Set the fd to non-bloquant
	int flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

Connection::~Connection()
{
	close(fd);
}

// Read "size" octets from the read buffer
// returns true and buf if enough datas can be read
// returns false when not enough data is available
bool Connection::Read(char **buf, size_t size)
{
	// Fill the buffer
	SocketRead();

	if(read_buf_size < size)
	{
		return false;
	}

	*buf = (char*)malloc(size);
	memcpy(*buf, read_buf, size);

	// The buffer has been totally read
	if(size == read_buf_size)
	{
		read_buf_size = 0;
		free(read_buf);
		read_buf = NULL;
		return true;
	}

	// Remove read data from the buffer
	read_buf_size -= size;
	char* new_buf = (char*)malloc(read_buf_size);
	memcpy(new_buf, read_buf + size, read_buf_size);
	free(read_buf);
	read_buf = new_buf;
	return true;
}

void Connection::Write(const char* buf, size_t size)
{
	write_buf = (char*)realloc(write_buf, write_buf_size + size);
	memcpy(write_buf + write_buf_size, buf, size);
	write_buf_size += size;

	// Try to flush to the socket
	SocketWrite();
}
