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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <exception>

class Connection
{
	int fd;
protected:
	char* read_buf;
	size_t read_buf_size;
public:
	class ConnectionError : public std::exception {};

	Connection(int _fd);
	virtual ~Connection() {}

	// Fill the buffer with incoming datas
	virtual void ReadToBuf() = 0;
	virtual void Write(const char* buf, size_t size) = 0;
	bool Read(char **buf, size_t size);
};
#endif						  // CONNECTION_H
