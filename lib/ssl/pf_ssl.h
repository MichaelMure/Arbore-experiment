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

#ifndef PF_SSL_H
#define PF_SSL_H

#include <string>
#include <map>
#include "pf_exception.h"
#include "connection.h"

class Ssl
{
protected:
	std::map<int, Connection*> fd_map;
public:
	class ConnectionError : public StrException
	{
		public:
			ConnectionError(std::string _error="Connection failed") : StrException(_error) {}
	};

	Ssl() {}
	virtual ~Ssl()			  /* Needed for abstract classes */
	{
	}

	Connection* GetConnection(int fd)
	{
		std::map<int, Connection*>::iterator c;
		c = fd_map.find(fd);

		if(c == fd_map.end())
			return NULL;

		return c->second;
	}

	virtual Connection* Accept(int fd) = 0;
	virtual Connection* Connect(int fd) = 0;
	virtual void Close(Connection* conn) = 0;
	virtual void CloseAll() = 0;
};
#endif						  // PF_SSL_H
