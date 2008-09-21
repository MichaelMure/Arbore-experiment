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
 * 
 */

#ifndef CRL_H
#define CRL_H
#include <string>
#include <openssl/x509.h>
#include "pf_exception.h"
#include "pf_thread.h"

class Crl					  /* : public Thread */
{
	std::string path;
	std::string url;
	X509_CRL* crl;
	bool disabled;
public:
	Crl();
	~Crl();

	class BadCRL : public StrException
	{
		public:
			BadCRL(std::string err) : StrException(err) {}
	};
	class DownloadFailed : public std::exception {};

	void Set(std::string _path, std::string _url) { path = _path; url = _url; }
	void Load();

	X509_CRL* GetSSL() { return crl; }
	void Disable() { disabled = true; }
	bool GetDisabled() const { return disabled; }

	void Loop();
};

extern Crl crl;
#endif						  /* CRL_H */
