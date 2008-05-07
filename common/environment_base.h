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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#ifndef ENVIRONMENT_BASE_H
#define ENVIRONMENT_BASE_H
#include "mutex.h"
#include "pf_types.h"

class EnvironmentBase
{
protected:
	template <typename A> class SharedVar : private Mutex
	{
		A var;
		public:
			SharedVar(A _var) : var(_var) {}
			~SharedVar() {}

			void Set(A _var)
			{
				BlockLockMutex lock(this);
				var = _var;
			}
			A Get()
			{
				BlockLockMutex lock(this);
				return var;
			}
	};
public:
	EnvironmentBase() : my_id(0), listening_port(0) {}
	SharedVar<pf_id> my_id;
	SharedVar<uint16_t> listening_port;
};
#endif						  /* ENVIRONMENT_BASE_H */
