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

#ifndef PF_SSL_NOSSL_H
#define PF_SSL_NOSSL_H

#include <string>
#include "pf_ssl.h"

class SslNoSsl : public Ssl
{
	int fd;

public:
	SslNoSsl();
	~SslNoSsl();

	void Bind(std::string interface, uint16_t port);
	std::list<Connection*> Select();

	Connection* Accept(int fd);
	Connection* Connect(int fd);
	void Connect(std::string host, uint16_t port);
	void Close(Connection* conn);
	void CloseAll();
};
#endif						  // PF_SSL_NOSSL_H
