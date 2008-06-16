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
 * $Id$
 */

#ifndef PF_SSL_SSL_H
#define PF_SSL_SSL_H

#include <openssl/ssl.h>
#include "pf_exception.h"
#include "pf_ssl.h"
#include "certificate.h"

class SslSsl : public Ssl
{
private:
	SSL_CTX* server_ctx;
	SSL_CTX* client_ctx;

	Certificate cert;
	Certificate cacert;
	PrivateKey key;

	void SetCertificates(SSL_CTX* ctx);
	void CheckPeerCertificate(SSL* ssl);
	void ForceDisconnect(SSL* ssl, int fd);
public:
	class SslHandshakeFailed : public StrException
	{
		public:
			SslHandshakeFailed(std::string err) : StrException(err) {}
	};

	SslSsl(std::string cert, std::string key, std::string ca);
	~SslSsl();

	Certificate GetCertificate() const { return cert; }
	Certificate GetCACertificate() const { return cacert; }

	Connection* Accept(int fd) throw(SslHandshakeFailed);
	Connection* Connect(int fd) throw(SslHandshakeFailed);
	void Close(Connection* conn);
	void CloseAll();
};
#endif						  // PF_SSLSSL_H
