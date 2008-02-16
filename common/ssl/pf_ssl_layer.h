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

#ifndef PF_SSL_LAYER_H
#define PF_SSL_LAYER_H

#include <list>
#include <exception>

class SslLayer
{
public:
	class ConnectionError : public std::exception {};

	virtual ~SslLayer() {} /* Needed for abstract classes */

	virtual void Bind(std::string interface, uint16_t port) = 0;
	virtual std::list<SslConnection*> Select() = 0;

	virtual void Connect(std::string host, uint16_t port) = 0;
	virtual void Close(SslConnection* conn) = 0;
	virtual void CloseAll() = 0;
};

#endif // PF_SSL_LAYER_H
