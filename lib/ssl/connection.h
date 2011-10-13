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
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include "pf_exception.h"
#include "pf_types.h"

/** This base class provide an abstraction for a connection, and have to be derived in concrete
  * implementation for e.g add encryption or not.  */
class Connection
{
private:
	int fd;

protected:
	char* read_buf;
	size_t read_buf_size;

	char* write_buf;
	size_t write_buf_size;

	/** This method fould fill the read buffer with incoming datas. */
	virtual void SocketRead() = 0;

	/** This method should send the data stored in the write buffer. */
	virtual void SocketWrite() = 0;

public:
	class RecvError : public StrException
	{
		public:
			RecvError(std::string err) : StrException(err) {}
	};
	class WriteError : public StrException
	{
		public:
			WriteError(std::string err) : StrException(err) {}
	};

	/** Constructor
	 * @param _fd File descriptor of the connection
	 */
	Connection(int _fd);

	virtual ~Connection();

	/** Copy size octet to the internal buffer
	 * @param buf pointer to the data
	 * @param size the size of the buffer
	 */
	void Write(const char* buf, size_t size);

	/** Copy size octet from the internal buffer
	 * @param buf pointer where is stored the newly allocated buffer
	 * @param size the number of octet asked to be read
	 *
	 * @return true if enough datas can be read, false otherwise
	 * \note If not enough data can be read, no data is removed from the
	 * internal buffer, and buf is not allocated.
	 */
	bool Read(char **buf, size_t size);

	/** @return the file descriptor of the connection. */
	int GetFd() const { return fd; }
};
#endif						  // CONNECTION_H
