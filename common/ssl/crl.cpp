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

#include <string.h>
#include <openssl/x509.h>
#include <openssl/safestack.h>
#include "crl.h"
#include "log.h"

Crl crl;

Crl::Crl() : filename(""), crl(NULL), disabled(false)
{
}

Crl::~Crl()
{

}

void Crl::Load(std::string _filename)
{
	log[W_INFO] << "Loading CRL file : \"" << _filename << "\".";

	if(crl)
	{
		X509_CRL_free(crl);
		crl = NULL;
	}

	// TODO: simplify-me with openssl's BIO
	FILE* f = fopen(_filename.c_str(), "r");
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

	BIO* raw_crl = BIO_new_mem_buf(buf, file_size);
	crl = PEM_read_bio_X509_CRL(raw_crl, NULL, NULL, NULL);
	BIO_free(raw_crl);

	if(!crl)
	{
		std::string str = std::string(ERR_error_string( ERR_get_error(), NULL));
		throw BadCRL(str);
	}

	char* str = X509_NAME_oneline (X509_CRL_get_issuer (crl), 0, 0);
	log[W_INFO] << "CRL issued by: " << str;
	filename = _filename;
}

