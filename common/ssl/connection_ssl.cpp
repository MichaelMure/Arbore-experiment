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
#include "log.h"

ConnectionSsl::~ConnectionSsl()
{
	SSL_shutdown(ssl);
	SSL_free(ssl);
}

void ConnectionSsl::SocketWrite()
{
	size_t written = SSL_write(ssl, write_buf, write_buf_size);
	if(written <= 0)
	{
		// No error, we are just waiting for datas
		// A WANT_READ can be returnd see man SSL_read()
		if(SSL_get_error(ssl, written) == SSL_ERROR_WANT_READ
			|| SSL_get_error(ssl, written) == SSL_ERROR_WANT_WRITE)
			return;
		log[W_DEBUG] << "Write failed";
		throw WriteError();
	}
	else
	if(written == write_buf_size)
	{
		// All the buffer has been sent
		write_buf_size = 0;
		free(write_buf);
		write_buf = NULL;
	}
	else
	{
		// Partial send
		write_buf_size -= written;
		char* new_buf = (char*)malloc(write_buf_size);
		memcpy(new_buf, write_buf + written, write_buf_size);
		free(write_buf);
		write_buf = new_buf;
	}
}

void ConnectionSsl::SocketRead()
{
	const int buf_size = 128;
	char buf[buf_size];

	int received = 0;

	do
	{
		received = SSL_read(ssl, (void*)buf, buf_size);
		if(received > 0)
		{
			read_buf = (char*)realloc(read_buf, read_buf_size + received);
			memcpy(read_buf + read_buf_size, buf, received);
			read_buf_size += received;
		}
		else
		if(received == 0)
		{
			throw RecvError();	  // TODO : return the SSL error string...
		}
		else				  // received < 0
		{
			// No error, we are just waiting for datas
			// A WANT_WRITE can be returnd see man SSL_read()
			if(SSL_get_error(ssl, received) == SSL_ERROR_WANT_READ
				|| SSL_get_error(ssl, received) == SSL_ERROR_WANT_WRITE)
				return;
			throw RecvError();	  // TODO : return the SSL error string...
		}
	}
	while(received == buf_size);
}
