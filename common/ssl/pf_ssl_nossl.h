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
 * $Id$
 */

#ifndef PF_SSL_NO_SSL_H
#define PF_SSL_NO_SSL_H

#include <list>
#include <exception>
#include "pf_ssl_layer.h"

class SslNoSsl : public SslLayer
{
	int fd;

public:
	class ConnectionError : public std::exception {};

	SsliNoSsl();

	void Bind(std::string interface, uint16_t port);
	std::list<SslConnection*> Select();

	void Connect(std::string host, uint16_t port);
	void Close(SslConnection* conn);
	void CloseAll();
};

#endif // PF_SSL_NO_SSL_H
