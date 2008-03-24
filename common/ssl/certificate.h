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

#ifndef PF_CERTIFICATE_H
#define PF_CERTIFICATE_H
#include <exception>
#include "pf_exception.h"

#include "public_key.h"
#include "private_key.h"
#include "pf_types.h"

class Certificate
{
private:
	X509* ssl_cert;

	void LoadX509Buf(const char* buf, size_t size);
public:
	class BadCertificate : public StrException
	{
		public:
			BadCertificate(std::string _error) : StrException(_error) {}
	};

	class BadFile : public std::exception {};

	Certificate();
	Certificate(const Certificate& cert);
	Certificate& operator=(const Certificate& cert);
	~Certificate();

	void LoadPem(std::string filename, std::string password) throw(BadFile, BadCertificate);
	void LoadSSL(X509* _ssl_cert);
	void LoadRaw(const unsigned char* buf, size_t len) throw(BadCertificate);

	PublicKey GetPublicKey();

	const std::string GetCertificateInfos();
	pf_id GetIDFromCertificate() throw(BadCertificate);

	X509* GetSSL() { return ssl_cert; }
	void GetRaw(unsigned char** buf, size_t* len);
};
#endif						  // PF_CERTIFICATE_H
