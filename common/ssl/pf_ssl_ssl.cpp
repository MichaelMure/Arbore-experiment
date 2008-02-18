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

#include <stdio.h>
#include <list>
#include <exception>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include "pf_ssl_ssl.h"
#include "certificate.h"
#include "connection_ssl.h"

SslSsl::SslSsl()
{
	// TODO: handle return codes
	SSL_load_error_strings();

	SSLeay_add_ssl_algorithms();
	SSL_METHOD* meth = SSLv23_server_method();
	ssl_ctx = SSL_CTX_new(meth);

	if(SSL_CTX_use_certificate_file(ssl_ctx, "server-cert.pem", SSL_FILETYPE_PEM)	<= 0
	|| SSL_CTX_use_PrivateKey_file(ssl_ctx, "server-key.pem", SSL_FILETYPE_PEM)	<= 0
	|| SSL_CTX_check_private_key(ssl_ctx)	<= 0)
	{
		printf("Failed to initialize something\n");
		exit(EXIT_FAILURE);
	}

}

SslSsl::~SslSsl()
{
}

std::list<Connection*> SslSsl::Select()
{
}

void SslSsl::HandShake(int fd)
{
	// TODO: check errors
	SSL* ssl = SSL_new(ssl_ctx);
	SSL_set_fd(ssl, fd);
	SSL_accept(ssl);

//	X509* client_cert = SSL_get_peer_certificate (ssl);
//	char* str = X509_NAME_oneline (X509_get_subject_name (client_cert), 0, 0);
//	printf ("\t subject: %s\n", str);
//	OPENSSL_free (str);
}

void SslSsl::Connect(std::string host, uint16_t port)
{
}

void SslSsl::Close(Connection* conn)
{
}

void SslSsl::CloseAll()
{
}


