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

#include <list>
#include <exception>
#include "pf_ssl_nossl.h"
#include "connection_nossl.h"

SslNoSsl::SslNoSsl()
{
}

SslNoSsl::~SslNoSsl()
{
}

Connection* SslNoSsl::Accept(int fd)
{
	// TODO: check errors
#if 0
	SSL* ssl = SSL_new(server_ctx);
	SSL_set_fd(ssl, fd);
	SSL_accept(ssl);
#endif

	Connection* new_conn = new ConnectionNoSsl(fd);
	fd_map[fd] = new_conn;

	return new_conn;
}

Connection* SslNoSsl::Connect(int fd)
{
#if 0
	SSL* ssl = SSL_new(client_ctx);
	SSL_set_fd(ssl, fd);
	SSL_connect(ssl);
#endif

	Connection* new_conn = new ConnectionNoSsl(fd);
	fd_map[fd] = new_conn;
	return new_conn;
}

void SslNoSsl::Close(Connection* conn)
{
}

void SslNoSsl::CloseAll()
{
}
