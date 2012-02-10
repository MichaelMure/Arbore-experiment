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

#ifndef KEY_H
#define KEY_H

#include <limits.h>
#include <set>
#include <stdint.h>
#include <string>
#include <util/pf_log.h>

/** Size of the Key in bits */
#define KEY_SIZE 160
/** Number of bits represented by an hexadecimal char */
#define HEXA_BASE 4
/** Size of the hexa representation of the key in char */
#define HEXA_KEYLENGTH KEY_SIZE / HEXA_BASE

class Key
{
public:
	/** Size of the key in char */
	static const size_t size = (KEY_SIZE / 8);
	/** Size of the key in uint32_t */
	static const size_t nlen = (KEY_SIZE / (8 * sizeof(uint32_t)));

	static const Key Key_Max;
	static const Key Key_Half;

	/** Create and return a random key.
	  * Be sure to have initialized the random number generator once
	  * before, using srand(time(NULL))
	  */
	static Key GetRandomKey();

	/** Create a Key from an uint32_t.
	 *
	 * @param ul this is the last 32 bits of the key
	 */
	Key(uint32_t ul = 0);

	/** Create a Key from a serialized key
	 *
	 * @param buf is the key serialized
	 */
	Key(const char* buf);

	/** Create a Key from a uint32_t[nlen] */
	explicit Key(uint32_t key[nlen]);

	/** Create a Key from a std::string */
	explicit Key(std::string str);

	/** The copy constructor.
	 *
	 * @param k2 copy key from the other key
	 */
	Key(const Key& k2);

	/** Parse a key from an hexa string (0x1123456ABE and 1123456ABE supported) */
	Key& operator=(const char* str);

	/** Parse a key from an hexa string (0x1123456ABE and 1123456ABE supported) */
	Key& operator=(std::string str);

	/** Copy a key from an other key
	 *
	 * @param k2 key used to be copied from.
	 * @return a new Key object.
	 */
	Key& operator=(const Key& k2);

	/** Comparaison between two keys.
	 *
	 * @param k2 other key which is compared to.
	 * @return true if two keys are equals.
	 */
	bool operator==(const Key& k2) const;

	/** Comparaison with an other key.
	 *
	 * @param k2 other key
	 * @return true if two keys are *NOT* equals.
	 */
	bool operator!=(const Key& k2) const { return !(*this == k2); }

	/** Comparaison with an other key.
	 *
	 * @param k2 other key which is compared to.
	 * @return true if I'm superior than k2.
	 */
	bool operator>(const Key& k2) const;
	bool operator<(const Key& k2) const;

	/** Return the string hexadecimal representation of key. */
	std::string GetStr() const;

	const uint32_t* GetArray() const { return t; }

	/** Assign sha1 hash of the string
	 *
	 * @param s hashed string.
	 */
	void MakeHash (std::string s);

	/** Asign sha1 hash of the string
	 *
	 * @param s hashed string
	 * @param size size of the string (if this isn't a 0 terminated string).
	 */
	void MakeHash (const char *s, size_t size);

	/** Calculate the distance between this and another key
	*
	* @param k2 key that we want to evaluate the distance
	* @return difference between this and k2
	*/
	Key distance(const Key& k2) const;

	/** Check if the key is between 2 others keys
	*
	* check to see if the value of this falls in the range from left clockwise
	* around the ring to right.
	*
	* @param left key which will be considered on the left in keyspace
	* @param right key which will be considered on the right in keyspace
	* @return true if the key is between or on one edge
	* */
	bool between (const Key& left, const Key& right) const;

	/** Check if the key is null.
	 * @return true if the key is equal to 0.
	 */
	bool operator!() const;

	/** Check if the key is non-null.
	 * @return true if the key is different than 0.
	 */
	operator bool() const;

	/** Calculate the lenght of the longest prefix match between this and a key
	*
	* @param key you wan't compare prefix with this
	* @return size of the prefix match
	*/
	size_t key_index (Key k) const;

	/** Serialyze the key in binary format */
	void dump(char* buf) const;

private:
	uint32_t t[nlen];

	/** Static function which initialize the maximum and half keys. */
	static Key Init_Max();
	static Key Init_Half();

	Key operator+(const Key& op2) const;
	Key operator-(const Key& k2) const;
};

template<>
inline Log::flux& Log::flux::operator<< <Key> (Key key)
{
	_str += key.GetStr();
	return *this;
}

typedef std::set<Key> KeyList;

#endif /* KEY_H */
