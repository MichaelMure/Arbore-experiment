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

#ifndef PF_ID_H
#define PF_ID_H

#include <ostream>

class PublicKey;

class pf_id
{
	static const size_t IdLength = 160; /**< bits */
	static const size_t nlen = IdLength / (8*sizeof(char));

	char hash[nlen];

public:

	/** Create a random pf_id */
	static pf_id FromRandom();

	/** Create a pf_id from a public key.
	 * It'll do a SHA-1 of it.
	 */
	static pf_id FromPublicKey(const PublicKey& key);

	pf_id();
	~pf_id();

	pf_id(const pf_id&);
	pf_id& operator=(const pf_id&);

	/** hash must be an unsigned char[nlen] */
	pf_id(const unsigned char* hash);
	pf_id& operator=(const unsigned char* hash);

	bool operator==(const pf_id&);

	std::string toString() const;

};

std::ostream& operator<<(std::ostream&, const pf_id&);

#endif /* PF_ID_H */