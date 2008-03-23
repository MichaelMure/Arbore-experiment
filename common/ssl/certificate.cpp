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

Certificate::Certificate() : ssl_cert(NULL)
{
}

Certificate::Certificate(const Certificate& cert)
			: ssl_cert(NULL)
{
	if(cert.ssl_cert != NULL)
		ssl_cert = X509_dup(cert.ssl_cert);
}

Certificate& Certificate::operator=(const Certificate& cert)
{
	if(cert.ssl_cert != NULL)
		ssl_cert = X509_dup(cert.ssl_cert);
	else
		ssl_cert = NULL;

	return *this;
}

Certificate::~Certificate()
{
	if(ssl_cert)
		X509_free(ssl_cert);
}

void Certificate::LoadPem(std::string filename, std::string password)
{
	if(ssl_cert)
	{
		X509_free(ssl_cert);
		ssl_cert = NULL;
	}

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

	BIO* raw_cert_bio = BIO_new_mem_buf(buf, file_size);
	ssl_cert = PEM_read_bio_X509(raw_cert_bio, NULL, NULL, NULL);
	BIO_free(raw_cert_bio);

	if(!ssl_cert)
		throw BadCertificate();
}

void Certificate::LoadSSL(X509* _ssl_cert)
{
	if(ssl_cert)
		X509_free(ssl_cert);

	/* XXX do we use the same object (like here), or we can copy
	 * certificate with X509_dup()?
	 */
	ssl_cert = _ssl_cert;
}

void Certificate::LoadRaw(const unsigned char* buf, size_t len)
{
	if(ssl_cert)
		X509_free(ssl_cert);

	ssl_cert = d2i_X509(NULL, &buf, len);

	if (ssl_cert == NULL)
		throw BadCertificate();
	/* Some error */
}

void Certificate::GetRaw(unsigned char** buf, size_t* len)
{
	// Returns a certificate in binary format
	// Non-tested
	unsigned char *p;

	*len = (size_t)i2d_X509(ssl_cert, NULL);

	*buf = (unsigned char*)malloc(*len);
	p = *buf;

	i2d_X509(ssl_cert, &p);
}

pf_id Certificate::GetIDFromCertificate()
{
	if(!ssl_cert)
		return 0;

	ASN1_INTEGER* no = X509_get_serialNumber(ssl_cert);
	long id = ASN1_INTEGER_get(no);

	/* Note: it appears to free part of memory of ssl_cert,
	 * so a call to X509_cert() will crash. -romain
	 */
	//M_ASN1_INTEGER_free(no);

	if(id <= 0 || (unsigned long)id > ID_MAX)
		throw BadCertificate();

	return id;
}
