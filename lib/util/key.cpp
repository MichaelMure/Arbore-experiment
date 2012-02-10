/*
 * Copyright(C) 2008 Romain Bignon
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
 * This file contains some code from the Chimera's Distributed Hash Table,
 * written by CURRENT Lab, UCSB.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <cassert>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include "key.h"
#include <util/pf_log.h>

const Key Key::Key_Max = Init_Max();
const Key Key::Key_Half = Init_Half();

Key Key::GetRandomKey()
{
	uint32_t key[nlen];

	for(size_t i = 0; i < nlen; ++i)
	{
		key[i] = 0;
		for(int j = 0; j < 32; j++)
		{
			key[i] += (rand()%2) << j;
		}
	}

	return Key(key);
}

Key::Key(uint32_t ul)
{
	size_t i;
	for (i = 1; i < nlen; i++)
		this->t[i] = 0;
	this->t[0] = ul;
}

Key::Key(const char* buf)
{
	for(size_t i = 0; i < Key::nlen; ++i)
	{
		t[i] = ntohl(*(uint32_t*)buf);
		buf += sizeof(t[i]);
	}
}

Key::Key(uint32_t key[Key::nlen])
{
	size_t i;
	for(i = 0; i < nlen; ++i)
		this->t[i] = key[i];
}

Key::Key(std::string str)
{
	*this = str.c_str();
}

Key::Key(const Key& k2)
{
	size_t i;
	for (i = 0; i < nlen; i++)
		this->t[i] = k2.t[i];
}

Key& Key::operator= (const char *strOrig)
{
	size_t i, len;
	char key_str[HEXA_KEYLENGTH + 1];

	char str[HEXA_KEYLENGTH + 1];

	// This loop below is required, though Patrik L. from sparta recommended against it
	for (i = 0; i < HEXA_KEYLENGTH + 1; i++)
		key_str[i] = '0';
	memset (str, 0, HEXA_KEYLENGTH + 1);
	if (strlen (strOrig) < HEXA_KEYLENGTH)
	{
		strcpy (str, strOrig);
	}
	else
	{
		strncpy (str, strOrig, HEXA_KEYLENGTH);
		str[HEXA_KEYLENGTH] = '\0';
	}

	// By now, the string should be in base 16
	len = strlen (str);
	if (len == 0)
	{
		pf_log[W_ERR] << "str_to_key: Warning:Empty input string";
	}
	else if (len > HEXA_KEYLENGTH)
	{
		strncpy (key_str, str, HEXA_KEYLENGTH);
		//  key_str[KEY_SIZE/BASE_B]='\0';
	}

	else if (len <= HEXA_KEYLENGTH)
	{
		for (i = 0; i < len; i++)
			key_str[i + (HEXA_KEYLENGTH) - len] = str[i];
	}

	key_str[HEXA_KEYLENGTH] = '\0';

	for (i = 0; i < nlen; i++)
		sscanf (key_str + (i * 8 * sizeof (char)), "%08x", &(this->t[(nlen-1 - i)]));

	return *this;
}

Key& Key::operator=(std::string str)
{
	return Key::operator=(str.c_str());
}

Key& Key::operator=(const Key& k2)
{
	size_t i;
	for (i = 0; i < nlen; i++)
		this->t[i] = k2.t[i];

	return *this;
}

bool Key::operator==(const Key& k2) const
{
	size_t i;
	for (i = 0; i < nlen; i++)
		if (this->t[i] != k2.t[i])
			return false;
	return true;
}

bool Key::operator!() const
{
	for(size_t i = 0; i < nlen; ++i)
		if(t[i] > 0)
			return false;

	return true;
}

bool Key::operator>(const Key& k2) const
{
	for (int i = nlen-1; i >= 0; i--)
	{
		if (this->t[i] > k2.t[i])
			return true;
		else if (this->t[i] < k2.t[i])
			return false;
	}
	return false;
}

bool Key::operator<(const Key& k2) const
{
	for (int i = nlen-1; i >= 0; i--)
	{
		if (this->t[i] < k2.t[i])
			return true;
		else if (this->t[i] > k2.t[i])
			return false;
	}
	return false;
}

std::string Key::GetStr() const
{
	char keystr[HEXA_KEYLENGTH + 1] = {0};

	sprintf (keystr, "0x%08x%08x%08x%08x%08x",
		    (unsigned int) this->t[4], (unsigned int) this->t[3],
		    (unsigned int) this->t[2], (unsigned int) this->t[1],
		    (unsigned int) this->t[0]);

	return std::string(keystr);
}

Key::operator bool() const
{
	for(size_t i = 0; i < nlen; ++i)
		if(t[i] > 0)
			return true;
	return false;
}

void Key::MakeHash (std::string s)
{
	MakeHash(s.c_str(), s.size());
	pf_log[W_DEBUG] << "Key::MakeHash: HASH(" << s << ") = [" << *this << "]";
}

void Key::MakeHash (const char *s, size_t size)
{
	unsigned char *digest = new unsigned char[SHA_DIGEST_LENGTH];

	SHA1((const unsigned char*) s, size, digest);

	unsigned char *p = digest;
	for(size_t i = 0; i < Key::nlen; ++i)
	{
		t[i] = ntohl(*(uint32_t*)p);
		p += sizeof(t[i]);
	}
	delete[] digest;
}

Key Key::distance(const Key& k2) const
{
	Key diff;

	if (*this > k2)
		diff = *this - k2;
	else
		diff = k2 - *this;

	if (diff > Key_Half)
		diff = Key_Max - diff;

	return diff;
}

bool Key::between (const Key& left, const Key& right) const
{
	int complr = left < right;
	int complt = left < *this;
	int comptr = *this < right;

	/* it's on one of the edges */
	if (complt == 0 || comptr == 0)
		return true;

	if (complr < 0)
	{
		if (complt < 0 && comptr < 0)
			return true;
		return false;
	}
	else if (complr == 0)
	{
		return false;
	}
	else
	{
		if (complt < 0 || comptr < 0)
			return true;
		return false;
	}
}

size_t Key::key_index (Key k) const
{
	size_t max_len, i;
	std::string mystr, kstr;

	max_len = HEXA_KEYLENGTH;
	mystr = this->GetStr();
	kstr = k.GetStr();

	for (i = 0; (mystr[i] == kstr[i]) && (i < max_len); i++)
		;

	if (i == max_len)
		i = max_len - 1;

	pf_log[W_DEBUG] << "key_index:" << i;
	pf_log[W_DEBUG] << "me:" << *this;
	pf_log[W_DEBUG] << "lookup_key:" << k;

	return (i);
}

void Key::dump(char* p) const
{
	for(size_t i = 0; i < Key::nlen; ++i)
	{
		uint32_t nbr = htonl(t[i]);
		memcpy(p, &nbr, sizeof(nbr));
		p += sizeof(nbr);
	}
}

Key Key::Init_Max ()
{
	Key max;
	for (size_t i = 0; i < nlen; i++)
	{
		max.t[i] = UINT_MAX;
	}
	return max;
}

Key Key::Init_Half ()
{
	Key half;
	for (size_t i = 0; i < nlen; i++)
	{
		half.t[i] = UINT_MAX;
	}
	half.t[nlen-1] = half.t[nlen-1] / 2;
	return half;
}

Key Key::operator+(const Key& op2) const
{
	Key result;
	double tmp, a, b;
	size_t i;
	a = b = tmp = 0;

	for (i = 0; i < nlen; i++)
	{
		a = this->t[i];
		b = op2.t[i];
		tmp += a + b;

		if (tmp > ULONG_MAX)
		{
			result.t[i] = (uint32_t) tmp;
			tmp = 1;
		}
		else
		{
			result.t[i] = (uint32_t) tmp;
			tmp = 0;
		}
	}

	return result;
}

Key Key::operator-(const Key & op2) const
{
	Key result;
	size_t i;
	double tmp, a, b, carry;

	carry = 0;

	if (*this < op2)
	{
		pf_log[W_ERR] << "key_sub: Operation is not allowed " << GetStr() << " < " << op2.GetStr();
		return result;
	}

	for (i = 0; i < nlen; i++)
	{
		a = this->t[i] - carry;
		b = op2.t[i];

		if (b <= a)
		{
			 tmp = a - b;
			 carry = 0;
		}
		else
		{
			a = a + (double)UINT_MAX + 1;
			tmp = a - b;
			carry = 1;
		}
		result.t[i] = (uint32_t) tmp;
	}
	return result;
}
