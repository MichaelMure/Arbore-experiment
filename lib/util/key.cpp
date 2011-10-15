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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <cassert>
#include <openssl/evp.h>
#include "key.h"
#include <util/pf_log.h>

Key Key_Max;
Key Key_Half;

static void convert_base16 (unsigned char num, char *out)
{
    unsigned char mask = 15;
    int i = 0;
    for (i = 1; i >= 0; i--)
	{
	    int digit = num >> (i * 4);
	    sprintf (out, "%x", digit & mask);
	    out++;
	}
    *out = '\0';
}

void Key::set_key_str() throw()
{
	char keystr[KEY_SIZE / BASE_B + 1] = {0};
	/*
	sprintf (keystr, "0x%08x%08x%08x%08x%08x",
		    (unsigned int) this->t[4], (unsigned int) this->t[3],
		    (unsigned int) this->t[2], (unsigned int) this->t[1],
		    (unsigned int) this->t[0]);*/
	sprintf(keystr, "0x%08x", this->t[0]);

	this->key_str = keystr;
}

Key& Key::operator=(std::string str)
{
	return Key::operator=(str.c_str());
}

Key::Key(std::string str)
{
	*this = str.c_str();
}

Key& Key::operator= (const char *strOrig)
{
	size_t i, len;
	char key_str[KEY_SIZE / BASE_B + 1];

	char str[KEY_SIZE / BASE_B + 1];

	// This loop below is required, though Patrik L. from sparta recommended against it
	for (i = 0; i < KEY_SIZE / BASE_B + 1; i++)
		key_str[i] = '0';
	memset (str, 0, KEY_SIZE / BASE_B + 1);
	if (strlen (strOrig) < KEY_SIZE / BASE_B)
	{
		strcpy (str, strOrig);
	}
	else
	{
		strncpy (str, strOrig, KEY_SIZE / BASE_B);
		str[KEY_SIZE / BASE_B] = '\0';
	}

	// By now, the string should be in base 16
	len = strlen (str);
	if (len == 0)
	{
		pf_log[W_ERR] << "str_to_key: Warning:Empty input string";
	}
	else if (len > BASE_16_KEYLENGTH)
	{
		strncpy (key_str, str, BASE_16_KEYLENGTH);
		//  key_str[KEY_SIZE/BASE_B]='\0';
	}

	else if (len <= BASE_16_KEYLENGTH)
	{
		for (i = 0; i < len; i++)
			key_str[i + (BASE_16_KEYLENGTH) - len] = str[i];
	}

	key_str[BASE_16_KEYLENGTH] = '\0';

	for (i = 0; i < nlen; i++)
		sscanf (key_str + (i * 8 * sizeof (char)), "%08x", &(this->t[(nlen-1 - i)]));

	set_key_str();

	return *this;
}

Key::Key(const Key& k2) throw()
{
	size_t i;
	for (i = 0; i < nlen; i++)
		this->t[i] = k2.t[i];

	set_key_str();
}

Key& Key::operator=(const Key& k2)
{
	size_t i;
	for (i = 0; i < nlen; i++)
		this->t[i] = k2.t[i];

	set_key_str();

	return *this;
}

Key::Key(uint32_t ul) throw()
{
	size_t i;
	for (i = 1; i < nlen; i++)
		this->t[i] = 0;
	this->t[0] = ul;

	set_key_str();
}

Key::Key(uint32_t key[Key::nlen]) throw()
{
	size_t i;
	for(i = 0; i < nlen; ++i)
		this->t[i] = key[i];

	set_key_str();
}

Key& Key::operator=(uint32_t ul)
{
	size_t i;
	for (i = 1; i < nlen; i++)
		this->t[i] = 0;
	this->t[0] = ul;

	set_key_str();

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

bool Key::operator==(uint32_t ul) const
{
	if (t[0] != ul)
		return false;
	for (size_t i = 1; i < nlen; i++)
		if (t[i] != 0)
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

bool Key::operator!() const
{
	for(size_t i = 0; i < nlen; ++i)
		if(t[i] > 0)
			return false;

	return true;
}

Key::operator bool() const
{
	for(size_t i = 0; i < nlen; ++i)
		if(t[i] > 0)
			return true;
	return false;
}
Key Key::operator-(const Key & op2) const
{
	Key result;
	size_t i;
	double tmp, a, b, carry;

	carry = 0;

	if (*this < op2)
	{
		pf_log[W_ERR] << "key_sub: Operation is not allowed " << this->key_str << " < " << op2.key_str;
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

void Key::sha1_keygen (const char *key, size_t digest_size, char *digest) const
{
	EVP_MD_CTX mdctx;
	const EVP_MD *md;
	unsigned char *md_value;
	unsigned int md_len;
	size_t i;
	char digit[10];
	char *tmp;

	md_value = (unsigned char *) malloc (EVP_MAX_MD_SIZE);

	OpenSSL_add_all_digests ();

	md = EVP_get_digestbyname ("sha1");

	EVP_MD_CTX_init (&mdctx);
	EVP_DigestInit_ex (&mdctx, md, NULL);
	EVP_DigestUpdate (&mdctx, key, digest_size);
	EVP_DigestFinal_ex (&mdctx, md_value, &md_len);
	EVP_MD_CTX_cleanup (&mdctx);

	digest = (char *) malloc (KEY_SIZE / BASE_B + 1);

	tmp = digest;
	*tmp = '\0';
	for (i = 0; i < md_len; i++)
	{
		convert_base16 (md_value[i], digit);

		strcat (tmp, digit);
		tmp = tmp + strlen (digit);
	}

	free (md_value);

	tmp = '\0';
}

void Key::MakeHash (std::string s)
{
	MakeHash(s.c_str(), s.size());

	pf_log[W_DEBUG] << "key_makehash: HASH( " << s << "  ) = [" << *this << "]";
}

void Key::MakeHash (const char *s, size_t size)
{
	char digest[KEY_SIZE / BASE_B + 1];

	sha1_keygen (s, size, digest);
	*this = digest;
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

Key Key::intervalSize(const Key& upperBound) const
{
	//the 0 key is not between the 2 bounds, easy
	if(*this < upperBound)
	{
		return upperBound - *this;
	}
	//0 key is between, split the interval into [lowerBound, keyMax][0,upperBound]
	return (Key_Max - upperBound) + *this;
}

bool Key::between (const Key& left, const Key& right) const
{
  assert(left < right);

  /* it's on one of the edges */
  if (left == *this || right == *this)
    return true;

  return (left < *this && *this < right);
}

Key Key::midpoint () const
{
	Key mid;
	if (*this < Key_Half)
		mid = *this + Key_Half;
	else
		mid = *this - Key_Half;

	return mid;
}

size_t Key::key_index (Key k) const
{
	size_t max_len, i;
	std::string mystr, kstr;

	max_len = KEY_SIZE / BASE_B;
	mystr = this->str();
	kstr = k.str();

	for (i = 0; (mystr[i] == kstr[i]) && (i < max_len); i++)
		;

	if (i == max_len)
		i = max_len - 1;

	pf_log[W_DEBUG] << "key_index:" << i;
	pf_log[W_DEBUG] << "me:" << *this;
	pf_log[W_DEBUG] << "lookup_key:" << k;

	return (i);
}

void Key::Init ()
{
	for (size_t i = 0; i < nlen; i++)
	{
		Key_Max.t[i] = UINT_MAX;
		Key_Half.t[i] = UINT_MAX;
	}
	Key_Half.t[4] = Key_Half.t[4] / 2;

	Key_Max.set_key_str();
	Key_Half.set_key_str();
}

