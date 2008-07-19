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

#ifndef PF_TYPES_H
#define PF_TYPES_H

#include <set>
#include <vector>
#include <stdint.h>

typedef unsigned int uint32_t;
typedef uint32_t pf_id;

/* Result of 2^32 */
#define ID_MAX 4294967295U

class pf_addr
{
public:
	uint32_t ip[4];
	uint16_t port;
	pf_id id;

	pf_addr() : port(0), id(0)
	{
		ip[0] = 0;
		ip[1] = 0;
		ip[2] = 0;
		ip[3] = 0;
	}

	bool operator ==(const pf_addr &other)
	{
		return ip[0] == other.ip[0]
			&& ip[1] == other.ip[1]
			&& ip[2] == other.ip[2]
			&& ip[3] == other.ip[3]
			&& port == other.port
			&& id == other.id;
	}
};

typedef std::vector<pf_addr> AddrList;
typedef std::set<pf_id> IDList;

/** Auto-deleter class for dynamical objects
 *
 * This class will free a specified object when it will
 * deleted.
 *
 * It's usefull to be sure that an object is free in all cases
 * when we leave a block (with exception, inside returns, etc.)
 */
template<typename T>
class Deleter
{
	T* p;
	Deleter(const Deleter<T>&);
	Deleter<T>& operator=(const Deleter<T>&);

public:
	explicit Deleter(T* _p)
		: p(_p)
		{}
	T* operator*() { return p; }
	~Deleter()
	{
		delete p;
	}
};
#endif						  /* PF_TYPES_H */
