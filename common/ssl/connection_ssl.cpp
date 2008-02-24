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

#include <openssl/ssl.h>
#include "connection_ssl.h"

ConnectionSsl::~ConnectionSsl()
{
	SSL_free(ssl);
}

void ConnectionSsl::Write(const char *buf, size_t size)
{
	SSL_write(ssl, buf, size);
}

void ConnectionSsl::ReadToBuf()
{
	const int buf_size = 128;
	int received = 0;
	char* buf = (char*)malloc(buf_size);
	
	do
	{
		received = SSL_read(ssl, (void*)buf, buf_size);
		if(received > 0)
		{
			read_buf = (char*)realloc(read_buf, buf_size + received);
			memcpy(read_buf + buf_size, buf, received);
			read_buf_size += received;
		}
	}
	while(received == buf_size);

	free(buf);
}

