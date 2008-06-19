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
 * $Id$
 */

#include "pf_id.h"

pf_id::pf_id()
{

}

pf_id::~pf_id()
{

}

pf_id::pf_id(const pf_id& id)
{
	for(size_t i = 0; i < nlen; ++i)
		hash[i] = id.hash[i];
}

pf_id& pf_id::operator=(const pf_id& id)
{
	for(size_t i = 0; i < nlen; ++i)
		hash[i] = id.hash[i];
}

bool pf_id::operator==(const pf_id& id)
{
	size_t i;
	for(i = 0; i < nlen && hash[i] == id.hash[i]; ++i);

	return (i >= nlen);
}

std::string pf_id::toString() const
{
	std::string s;
	for(size_t i = 0; i < nlen; ++i)
		for(size_t j = sizeof(hash[i]); j >= 0; --j)
			s += "0123456789abcdef"[((hash[i] >> j*4) & 0xF)];
	return s;
}

std::ostream& operator<<(std::ostream& os, const pf_id& id)
{
	os << id.toString();
	return os;
}

