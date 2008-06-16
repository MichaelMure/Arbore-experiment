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
 * $Id: private_key.cpp 1004 2008-05-07 22:51:21Z romain $
 */

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <cstring>
#include <climits>
#include "private_key.h"

PrivateKey::PrivateKey() : ssl_key(NULL), raw_key(NULL), raw_key_size(0)
{
}

PrivateKey::PrivateKey(const PrivateKey& pkey) : ssl_key(NULL), raw_key(NULL), raw_key_size(0)
{
	// TODO: really copy the key instead of reloading it
	if(pkey.raw_key)
		LoadBuf(pkey.raw_key, pkey.raw_key_size);
}

PrivateKey::~PrivateKey()
{
	if(raw_key) delete []raw_key;
	if(ssl_key) EVP_PKEY_free(ssl_key);
}

int PrivateKey::PasswordCallback(char* buf, int size, int rwflag, void* datas)
{
	buf[0] = '\0';
	return 0;
}

void PrivateKey::LoadBuf(const char* buf, size_t size) throw(BadPrivateKey)
{
	if(raw_key)
		delete []raw_key;

	raw_key = new char[size];
	raw_key_size = size;

	memcpy(raw_key, buf, size);

	if(ssl_key)
	{
		EVP_PKEY_free(ssl_key);
		ssl_key = NULL;
	}

	if(size > INT_MAX)
		throw BadPrivateKey(std::string("File is too big"));

	BIO* raw_key_bio = BIO_new_mem_buf(raw_key, (int)size);
	ssl_key = PEM_read_bio_PrivateKey(raw_key_bio, NULL, &PasswordCallback, NULL);
	BIO_free(raw_key_bio);

	if(!ssl_key)
	{
		raw_key = NULL;
		raw_key_size = 0;
		delete []raw_key;
		std::string str = std::string(ERR_error_string( ERR_get_error(), NULL));
		throw BadPrivateKey(str);
	}
}

void PrivateKey::LoadPem(std::string filename, std::string password) throw(BadPrivateKey, BadFile)
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

	LoadBuf(buf, file_size);
}
