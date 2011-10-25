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
 */

#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <cstring>
#include <climits>
#include "certificate.h"
#include <util/tools.h>

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

	if(file_size > UINT_MAX)
		throw BadFile();

	BIO* raw_cert_bio = BIO_new_mem_buf(buf, (uint32_t)file_size);
	ssl_cert = PEM_read_bio_X509(raw_cert_bio, NULL, NULL, NULL);
	BIO_free(raw_cert_bio);

	if(!ssl_cert)
	{
		std::string str = std::string(ERR_error_string( ERR_get_error(), NULL));
		throw BadCertificate(str);
	}
}

void Certificate::SetSSL(X509* _ssl_cert)
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

	if(!ssl_cert)
	{
		std::string str = std::string(ERR_error_string( ERR_get_error(), NULL));
		throw BadCertificate(str);
	}
}

void Certificate::GetRaw(unsigned char** buf, size_t* len) const
{
	// Returns a certificate in binary format
	// Non-tested
	unsigned char *p;
	int i = i2d_X509(ssl_cert, NULL);

	if(i < 0)
		throw BadCertificate(std::string(ERR_error_string(ERR_get_error(), NULL)));

	*len = (size_t) i;

	*buf = (unsigned char*)OPENSSL_malloc(*len);
	p = *buf;

	i2d_X509(ssl_cert, &p);
}

std::string Certificate::GetCommonName() const
{
	if(!ssl_cert)
		return "";

	X509_NAME *subject;
	int pos;
	X509_NAME_ENTRY *entry;
	ASN1_STRING *entry_str;
	int len;
	unsigned char *utf;
	std::string output;

	subject = X509_get_subject_name(ssl_cert);
	pos = X509_NAME_get_index_by_NID(subject, NID_commonName, -1);

	if (pos < 0)
		return "";

	if (X509_NAME_get_index_by_NID(subject, NID_commonName, pos) >= 0)
		return "";

	if ((entry = X509_NAME_get_entry(subject, pos)) == 0)
		throw BadCertificate("Unable to get common name");

	if ((entry_str = X509_NAME_ENTRY_get_data(entry)) == 0)
		throw BadCertificate("Unable to get common name");

	/* Canonicalize to UTF-8 and validate */
	len = ASN1_STRING_to_UTF8(&utf, entry_str);

	output = TypToStr(utf);

	if(utf)
		OPENSSL_free(utf);

	return output;
}
