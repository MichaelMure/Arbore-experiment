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
#include <openssl/evp.h>
#include "key.h"
#include "base.h"
#include "pf_log.h"

Key Key_Max;
Key Key_Half;

extern int power(int, int);

void Key::key_print ()
{
	char hexstr[KEY_SIZE];	// this is big just to be safe
	char base4str[KEY_SIZE];	//

	for (int i = nlen-1; i >= 0; i--)
		sprintf (hexstr, "%08x", (unsigned int) t[i]);

	if (IS_BASE_16)
	{
		for (size_t i = 0; i < strlen (hexstr); i++)
		{
			if (i % 8 == 0)
			printf (" ");
			printf ("%c", hexstr[i]);
		}
	}
	else if (IS_BASE_4)
	{
		hex_to_base4 (hexstr, base4str);

		for (size_t i = 0; i < strlen (base4str); i++)
		{
			if (i % 16 == 0)
			printf (" ");
			printf ("%c", base4str[i]);
		}
	}
	else
	{
		printf ("key.c: Unknown base \n");
	}
	printf ("\n");
}

void Key::key_to_str ()
{
	this->valid = 1;

	if (IS_BASE_16)
	{
		memset (this->keystr, 0, KEY_SIZE / BASE_B + 1);
		sprintf (this->keystr, "%08x%08x%08x%08x%08x",
		            (unsigned int) this->t[4], (unsigned int) this->t[3],
		            (unsigned int) this->t[2], (unsigned int) this->t[1],
		            (unsigned int) this->t[0]);
	}
	else if (IS_BASE_4)
	{
		char temp[KEY_SIZE];
		// if we need base4, then convert base16 to base4
		sprintf (temp, "%08x%08x%08x%08x%08x", (unsigned int) this->t[4],
			 (unsigned int) this->t[3], (unsigned int) this->t[2],
			 (unsigned int) this->t[1], (unsigned int) this->t[0]);

		hex_to_base4 (temp, this->keystr);
	}
	else
	{
		pf_log[W_ERR] << "key.c: Unknown base";
	}
}

Key& Key::operator= (const char *strOrig)
{
	size_t i, len;
	char key_str[KEY_SIZE / BASE_B + 1];

	char str[KEY_SIZE / BASE_B + 1];
	char tempString[KEY_SIZE];


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

	// Now, if str is in a different base than hex, replace the str contents with corresponding hex contents
	if (IS_BASE_4)
	{
		strcpy (tempString, str);
		memset (str, 0, strlen (tempString));
		base4_to_hex (tempString, str);
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

	for (i = 0; i < 5; i++)
		sscanf (key_str + (i * 8 * sizeof (char)), "%08x", &(this->t[(4 - i)]));

	key_to_str ();

	return *this;
}

Key::Key(const Key& k2)
{
	size_t i;
	for (i = 0; i < nlen; i++)
		this->t[i] = k2.t[i];

	key_to_str ();
}

Key& Key::operator=(const Key& k2)
{
	size_t i;
	for (i = 0; i < nlen; i++)
		this->t[i] = k2.t[i];

	key_to_str ();

	return *this;
}

Key::Key(uint32_t ul)
{
	size_t i;
	for (i = 1; i < nlen; i++)
		this->t[i] = 0;
	this->t[0] = ul;
	key_to_str ();
}

Key& Key::operator=(uint32_t ul)
{
	size_t i;
	for (i = 1; i < nlen; i++)
		this->t[i] = 0;
	this->t[0] = ul;
	key_to_str ();

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
	result.valid = 0;

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
		pf_log[W_ERR] << "key_sub: Operation is not allowed " << this->keystr << " < " << op2.keystr;
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

	result.valid = 0;

	return result;
}

char *sha1_keygen (char *key, size_t digest_size, char *digest)
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
		if (power (2, BASE_B) == BASE_16)
		{
			convert_base16 (md_value[i], digit);
		}
		else if (power (2, BASE_B) == BASE_4)
		{
			convert_base4 (md_value[i], digit);
		}
		else if (power (2, BASE_B) == BASE_2)
		{
			convert_base2 (md_value[i], digit);
		}

		strcat (tmp, digit);
		tmp = tmp + strlen (digit);
	}

	free (md_value);

	tmp = '\0';
	return (digest);
}

void Key::key_makehash (char *s)
{
	key_make_hash (s, strlen (s) * sizeof (char));

	pf_log[W_DEBUG] << "key_makehash: HASH( " << s << "  ) = ["
		     << get_key_string () << "]";
}

void Key::key_make_hash (char *s, size_t size)
{
	char *digest;

	digest = sha1_keygen (s, size, NULL);
	*this = digest;

	//for(i=0; i <5; i++) sscanf(digest+(i*8*sizeof(char)),"%08x",&hashed->t[(4-i)]);
	//key_to_str(hashed->keystr,*hashed);

	free (digest);
}

void Key::Init ()
{
	int i;
	for (i = 0; i < 5; i++)
	{
		Key_Max.t[i] = UINT_MAX;
		Key_Half.t[i] = UINT_MAX;
	}
	Key_Half.t[4] = Key_Half.t[4] / 2;

	Key_Max.key_to_str ();
	Key_Half.key_to_str ();

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

	diff.valid = 0;

	return diff;
}


int Key::between (const Key * const left, const Key * const right)
{

	int complr = *left < *right;
	int complt = *left < *this;
	int comptr = *this < *right;

	/* it's on one of the edges */
	if (complt == 0 || comptr == 0)
		return (1);


	if (complr < 0)
	{
		if (complt < 0 && comptr < 0)
			return (1);
		return (0);
	}
	else if (complr == 0)
	{
		return (0);
	}
	else
	{
		if (complt < 0 || comptr < 0)
			return (1);
		return (0);

	}
}

// Return the string representation of key
// This function should be used instead of directly accessing the keystr field
const char *Key::get_key_string ()
{
	if (!this->valid)
		key_to_str ();

	return this->keystr;
}

Key Key::midpoint () const
{
	Key mid;
	if (*this < Key_Half)
		mid = *this + Key_Half;
	else
		mid = *this - Key_Half;

	mid.valid = 0;

	return mid;
}

size_t Key::key_index (Key k)
{
	size_t max_len, i;
	char mystr[KEY_SIZE / BASE_B + 1];
	char kstr[KEY_SIZE / BASE_B + 1];

	max_len = KEY_SIZE / BASE_B;
	strcpy (mystr, this->get_key_string());
	strcpy (kstr, k.get_key_string());

	for (i = 0; (mystr[i] == kstr[i]) && (i < max_len); i++)
		;

	if (i == max_len)
		i = max_len - 1;

	pf_log[W_DEBUG] << "key_index:" << i;
	pf_log[W_DEBUG] << "me:" << this->keystr;
	pf_log[W_DEBUG] << "lookup_key:" << k.keystr;

	return (i);
}
