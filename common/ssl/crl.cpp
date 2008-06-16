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
 * $Id: crl.cpp 1138 2008-06-08 12:22:09Z romain $
 */

#include <string.h>
#include <openssl/x509.h>
#include <openssl/safestack.h>
#include "crl.h"
#include "pf_log.h"
#include "download.h"

Crl crl;

Crl::Crl() : path(""), crl(NULL), disabled(false)
{
}

Crl::~Crl()
{

}

void Crl::Load()
{
	if(!download.Get(url.c_str(), path.c_str()))
	{
		pf_log[W_INFO] << "CRL download failed";
		throw DownloadFailed();
	}
	pf_log[W_INFO] << "Loading CRL file : \"" << path << "\".";

	if(crl)
	{
		X509_CRL_free(crl);
		crl = NULL;
	}

	// TODO: simplify-me with openssl's BIO
	FILE* f = fopen(path.c_str(), "r");
	if(!f)
		throw BadCRL(std::string(strerror(errno)));

	if(fseek(f, 0, SEEK_END))
		throw BadCRL(std::string(strerror(errno)));

	size_t file_size = ftell(f);
	rewind(f);

	char buf[file_size];
	if(fread(buf, file_size, 1, f) <= 0)
	{
		fclose(f);
		throw BadCRL(std::string(strerror(errno)));
	}

	if(file_size > INT_MAX)
		throw BadCRL(std::string("File is too big"));

	BIO* raw_crl = BIO_new_mem_buf(buf, (int)file_size);
	crl = PEM_read_bio_X509_CRL(raw_crl, NULL, NULL, NULL);
	BIO_free(raw_crl);

	if(!crl)
	{
		std::string str = std::string(ERR_error_string( ERR_get_error(), NULL));
		throw BadCRL(str);
	}

	char* str = X509_NAME_oneline (X509_CRL_get_issuer (crl), 0, 0);
	pf_log[W_INFO] << "CRL issued by: " << str;
}

void Crl::Loop()
{

}
