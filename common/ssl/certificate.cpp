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

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <cstring>
#include "certificate.h"

Certificate::Certificate() : ssl_cert(NULL), raw_cert(NULL), raw_cert_size(0)
{
}

Certificate::Certificate(const Certificate& cert) : ssl_cert(NULL), raw_cert(NULL), raw_cert_size(0)
{
	if(cert.raw_cert)
		LoadX509Buf(cert.raw_cert, cert.raw_cert_size);
}

Certificate::~Certificate()
{
	if(raw_cert) delete []raw_cert;
	if(ssl_cert) X509_free(ssl_cert);
}

void Certificate::LoadX509Buf(const char* buf, size_t size)
{
	if(raw_cert)
		delete []raw_cert;

	raw_cert = new char[size];
	raw_cert_size = size;

	memcpy(raw_cert, buf, size);

	if(ssl_cert)
		X509_free(ssl_cert);

	BIO* raw_cert_bio = BIO_new_mem_buf(raw_cert, size);
	ssl_cert = PEM_read_bio_X509(raw_cert_bio, NULL, NULL, NULL);
	BIO_free(raw_cert_bio);

	if(!ssl_cert)
	{
		delete []raw_cert;
		throw BadCertificate();
	}
}

void Certificate::LoadX509(std::string filename, std::string password)
{
	FILE* f = fopen(filename.c_str(), "r");
	if(!f)
		throw BadFile();

	// Get file size
	if(fseek(f, 0, SEEK_END))
		throw BadFile();
	size_t file_size = ftell(f);
	rewind(f);

	// Read the file
	char buf[file_size];
	if(fread(buf, file_size, 1, f) <= 0)
		throw BadFile();
	fclose(f);

	LoadX509Buf(buf, file_size);
}
