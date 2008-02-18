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

#include <list>
#include <exception>
#include "pf_ssl_ssl.h"
#include "certificate.h"
#include "connection_ssl.h"

SslSsl::SslSsl()
{
}

SslSsl::~SslSsl()
{
}

void SslSsl::Bind(std::string interface, uint16_t port)
{
}

std::list<Connection*> SslSsl::Select()
{
}

void SslSsl::Connect(std::string host, uint16_t port)
{
}

void SslSsl::Close(Connection* conn)
{
}

void SslSsl::CloseAll()
{
}


