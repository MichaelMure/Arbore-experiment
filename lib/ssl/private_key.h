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

#ifndef PF_PRIVATE_KEY_H
#define PF_PRIVATE_KEY_H

#include <openssl/evp.h>
#include <string>
#include <util/pf_exception.h>

class PrivateKey
{
private:
	EVP_PKEY* ssl_key;
	char* raw_key;
	size_t raw_key_size;

public:
	class BadPrivateKey : public StrException
	{
		public:
			BadPrivateKey(std::string err) : StrException(err) {}
	};
	class BadPassword : public std::exception {};
	class BadFile : public std::exception {};

	PrivateKey();
	PrivateKey(const PrivateKey& pkey);
	~PrivateKey();

	static int PasswordCallback(char* buf, int size, int rwflag, void* datas);
	void LoadPem(std::string filename, std::string password) throw(BadPrivateKey, BadFile);
	void LoadBuf(const char* buf, size_t size) throw(BadPrivateKey);

	/* Not implemented yet */
	void Decrypt(const char* buf, size_t but_size, char** crypted, size_t* crypt_size);
	void Sign(const char* buf, size_t but_size, char** crypted, size_t* crypt_size);

	EVP_PKEY* GetSSL() { return ssl_key; }
};
#endif						  // PF_PRIVATE_KEY_H
