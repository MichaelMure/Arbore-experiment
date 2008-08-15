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

/*
** $Id: key.h,v 1.16 2006/06/07 09:21:28 krishnap Exp $
**
** Matthew Allen
** description:
*/

#ifndef _CHIMERA_KEY_H_
#define _CHIMERA_KEY_H_

#include <limits.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <string.h>

typedef unsigned int uint32_t;

#define KEY_SIZE 160
#define N_SIZE KEY_SIZE/sizeof(uint32_t)

// Changed this to 2 for base4 and 4 to have keys in base 16; Only these two are supported right now
#define BASE_B 4		/* Base representation of key digits */

#define BASE_16_KEYLENGTH 40

#define BASE_2 2
#define BASE_4 4
#define BASE_16 16

#define IS_BASE_2 (power(2, BASE_B) == BASE_2)
#define IS_BASE_4 (power(2, BASE_B) == BASE_4)
#define IS_BASE_16 (power(2, BASE_B) == BASE_16)

class Key
{
	static const size_t nlen = KEY_SIZE / (8 * sizeof(uint32_t));
	uint32_t t[nlen];
	char keystr[KEY_SIZE / BASE_B + 1];	/* string representation of key in hex */
	short int valid;		// indicates if the keystr is most up to date with value in key

	Key operator+(const Key& op2) const;
	Key operator-(const Key& k2) const;

public:

	static void Init();

	Key(uint32_t ul = 0);
	Key(const Key& k2);

	Key& operator=(const char* str);
	Key& operator=(const Key& k2);
	Key& operator=(uint32_t ul);
	bool operator==(const Key& k2) const;
	bool operator==(uint32_t ul) const;
	bool operator>(const Key& k2) const;
	bool operator<(const Key& k2) const;

	/* key_makehash: hashed, s
	** assign sha1 hash of the string #s# to #hashed# */

	void key_makehash (char *s);


	/* key_make_hash */
	void key_make_hash (char *s, size_t size);

	/* key_distance:k1,k2
	** calculate the distance between k1 and k2 in the keyspace and assign that to #diff# */
	Key distance(const Key& k2) const;

	/* key_between: test, left, right
	** check to see if the value in #test# falls in the range from #left# clockwise
	** around the ring to #right#. */

	int between (const Key * const left, const Key * const right);


	/* key_midpoint: mid, key
	** calculates the midpoint of the namespace from the #key#  */

	Key midpoint () const;


	/* key_index: mykey, key
	** returns the lenght of the longest prefix match between #mykey# and #k# */

	size_t key_index (Key k);

	void key_print ();

	void key_to_str ();

	const char* get_key_string ();	// always use this function to get the string representation of a key
};

/* global variables!! that are set in key_init function */
extern Key Key_Max;
extern Key Key_Half;

#endif /* _CHIMERA_KEY_H_ */
