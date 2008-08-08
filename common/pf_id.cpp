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

#include <stdlib.h>

#include "pf_id.h"

pf_id pf_id::FromRandom()
{
	unsigned char hash[nlen];
	for(size_t i = 0; i < nlen; ++i)
		hash[i] = rand();

	return pf_id(hash);
}

pf_id pf_id::FromPublicKey(const PublicKey& key)
{

}

pf_id::pf_id()
{
	for(size_t i = 0; i < nlen; ++i)
		hash[i] = 0;
}

pf_id::~pf_id()
{

}

pf_id::pf_id(const pf_id& id)
{
	for(size_t i = 0; i < nlen; ++i)
		hash[i] = id.hash[i];
}

pf_id::pf_id(const unsigned char* dhash)
{
	for(size_t i = 0; i < nlen; ++i)
		hash[i] = dhash[i];
}

pf_id& pf_id::operator=(const pf_id& id)
{
	for(size_t i = 0; i < nlen; ++i)
		hash[i] = id.hash[i];
}

pf_id& pf_id::operator=(const unsigned char* dhash)
{
	for(size_t i = 0; i < nlen; ++i)
		hash[i] = dhash[i];
}

bool pf_id::operator==(const pf_id& id)
{
	size_t i;
	for(i = 0; i < nlen && hash[i] == id.hash[i]; ++i);

	return (i >= nlen);
}

std::string pf_id::toString() const
{
	std::string s = "0x";
	for(size_t i = 0; i < nlen; ++i)
		for(int j = sizeof(hash[i]); j >= 0; --j)
			s += "0123456789abcdef"[((hash[i] >> j*4) & 0xF)];
	return s;
}

std::ostream& operator<<(std::ostream& os, const pf_id& id)
{
	os << id.toString();
	return os;
}

#ifdef TESTS
#include <iostream>
int main()
{
	pf_id id, id2;
	const unsigned char sha[20] = {128,1,28,83,37,112,234,12,48,89,254,8,87,209,221,187,37,123,211,0};

	std::cout << "Null id: " << id << std::endl;

	id = sha;
	std::cout << "Hash id: " << id << std::endl;

	srand(time(NULL));
	std::cout << "Rand id: " << pf_id::FromRandom() << std::endl;

	id2 = id;
	if(id == id2)
		std::cout << id << " = " << id2 << std::endl;
	else
		std::cout << id << " != " << id2 << std::endl;

	return 0;
}
#endif