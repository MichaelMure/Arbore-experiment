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
 * $Id: connection_ssl.cpp 1138 2008-06-08 12:22:09Z romain $
 */

#include <assert.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <climits>
#include "connection_ssl.h"
#include "pf_log.h"

ConnectionSsl::~ConnectionSsl()
{
	if(ssl)
	{
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}
}

void ConnectionSsl::SocketWrite() throw(WriteError)
{
	if(write_buf_size != 0)
	{
		buf_t new_buf;
		new_buf.buf = write_buf;
		new_buf.size = write_buf_size;
		write_buf = NULL;
		write_buf_size = 0;
		buf_queue.push(new_buf);
	}

	while(!buf_queue.empty())
	{
		if(buf_queue.front().size > INT_MAX)
			throw WriteError(std::string("Packet is too big"));
		int written = SSL_write(ssl, buf_queue.front().buf, (int)buf_queue.front().size);
		if(written < 0)
		{
			// No error, we are just waiting for datas
			// A WANT_READ can be returnd see man SSL_read()
			if(SSL_get_error(ssl, written) == SSL_ERROR_WANT_READ
				|| SSL_get_error(ssl, written) == SSL_ERROR_WANT_WRITE)
				return;
			pf_log[W_DEBUG] << "Write failed";
			std::string err = ERR_error_string(ERR_get_error(), NULL);
			throw WriteError(err);
		}
		else if(written == 0)
		{
			throw WriteError("Peer disconnected");
		}
		if(written == (int)buf_queue.front().size)
		{
			// All the buffer has been sent
			free(buf_queue.front().buf);
			buf_queue.pop();
		}
		else
		{
			// Partial send
			buf_queue.front().size -= written;
			char* new_buf = (char*)malloc(buf_queue.front().size);
			memcpy(new_buf, buf_queue.front().buf + written, buf_queue.front().size);
			free(buf_queue.front().buf);
			buf_queue.front().buf = new_buf;
			break;
		}
	}
}

void ConnectionSsl::SocketRead() throw(RecvError)
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
		else if(received == 0)
		{
			throw RecvError("Peer disconnected");
		}
		else
		{
			// No error, we are just waiting for datas
			// A WANT_WRITE can be returnd see man SSL_read()
			if(SSL_get_error(ssl, received) == SSL_ERROR_WANT_READ
				|| SSL_get_error(ssl, received) == SSL_ERROR_WANT_WRITE)
				return;
			std::string err = ERR_error_string( ERR_get_error(), NULL);
			throw RecvError(err);
		}
	}
	while(received == buf_size);
}

Certificate ConnectionSsl::GetCertificate()
{
	assert(ssl);
	X509* ssl_cert = SSL_get_peer_certificate(ssl);
	assert(ssl_cert);

	Certificate cert;
	cert.SetSSL(X509_dup(ssl_cert));
	return cert;
}

pf_id ConnectionSsl::GetCertificateID()
{
	return GetCertificate().GetIDFromCertificate();
}
