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
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include "log.h"
#include "pf_ssl_ssl.h"
#include "certificate.h"
#include "connection_ssl.h"

SslSsl::SslSsl(std::string cert_file, std::string key_file, std::string cacert_file) throw (Certificate::BadCertificate)
{
	// TODO: handle return codes
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();

	// Server part initialization
	SSL_METHOD* meth = SSLv23_server_method();
	server_ctx = SSL_CTX_new(meth);
	SSL_CTX_set_mode(server_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);

	// Client part initialization
	meth = SSLv23_client_method();
	client_ctx = SSL_CTX_new(meth);
	SSL_CTX_set_mode(client_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);

	log[W_INFO] << "Loading private key: " << key_file;
	key.LoadPem(key_file, "");
	log[W_INFO] << "Loading certificate: " << cert_file;
	cert.LoadPem(cert_file, "");
	log[W_INFO] << "Loading CA certificate: " << cacert_file;
	cacert.LoadPem(cacert_file, "");

	SetCertificates(server_ctx);
	SetCertificates(client_ctx);
}

void SslSsl::SetCertificates(SSL_CTX* ctx)
{
	int ret;
	// TODO: specify a callback that checks the peers cert without checking
	// the certificates purpose
	//SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

	// Load certificates in the session
	if((ret = SSL_CTX_use_certificate(ctx, cert.GetSSL()))  <= 0
		|| (ret = SSL_CTX_use_PrivateKey(ctx, key.GetSSL()))    <= 0
		|| (ret = SSL_CTX_check_private_key(ctx))               <= 0)
	{
		std::string str = std::string(ERR_error_string( ERR_get_error(), NULL));
		throw Certificate::BadCertificate(str);
	}

	// Set trust certificates
	X509_STORE* st = SSL_CTX_get_cert_store(ctx);
	X509_STORE_add_cert(st, cacert.GetSSL());
	// TODO: set the crl
}

SslSsl::~SslSsl()
{
}

Connection* SslSsl::Accept(int fd) throw(SslSsl::SslHandshakeFailed)
{
	int ret;
	SSL* ssl = SSL_new(server_ctx);
	SSL_set_fd(ssl, fd);
	if((ret = SSL_accept(ssl)) <= 0)
	{
		std::string err = ERR_error_string(SSL_get_error(ssl, ret), NULL);
		throw SslHandshakeFailed(err);
	}

	ConnectionSsl* new_conn = new ConnectionSsl(ssl, fd);
	fd_map[fd] = new_conn;
	//	X509* client_cert = SSL_get_peer_certificate (ssl);
	//	char* str = X509_NAME_oneline (X509_get_subject_name (client_cert), 0, 0);
	//	printf ("\t subject: %s\n", str);
	//	OPENSSL_free (str);

	return new_conn;
}

Connection* SslSsl::Connect(int fd) throw(SslSsl::SslHandshakeFailed)
{
	int ret;
	SSL* ssl = SSL_new(client_ctx);
	SSL_set_fd(ssl, fd);
	if((ret = SSL_connect(ssl)) <= 0)
	{
		std::string err = ERR_error_string(SSL_get_error(ssl, ret), NULL);
		throw SslHandshakeFailed(err);
	}

	ConnectionSsl* new_conn = new ConnectionSsl(ssl, fd);
	fd_map[fd] = new_conn;
	return new_conn;
}

void SslSsl::Close(Connection* conn)
{
}

void SslSsl::CloseAll()
{
}
