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

#ifndef PACKET_ARG_H
#define PACKET_ARG_H

#include <string>

enum PacketArgType
{
	T_UINT32,
	T_UINT64,
	T_STR,
	T_ADDR,
	T_ADDRLIST,
	T_IDLIST,
	T_CHUNK,
	T_CERTIFICATE,
	T_NONE
};

#define MAX_PACKET_ARGS 16
extern const PacketArgType packet_args[][MAX_PACKET_ARGS];

/* This is base class used to stock pointers */
class PacketArgBase
{
public:
	PacketArgBase() {}
	virtual ~PacketArgBase() {}

	virtual PacketArgBase* clone() const = 0;
};

/* Generic template class. We can't use it because it is abstract.
 * Please use one of specialization.
 */
template <class A> class PacketArg : public PacketArgBase
{
public:
	A val;

	PacketArg(A _val) { val = _val; }
	PacketArgBase* clone() const { return new PacketArg<A>(*this); }
};
#endif						  /* PACKET_ARG_H */
