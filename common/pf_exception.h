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
 * $Id$
 */

#ifndef PF_EXCEPTION_H
#define PF_EXCEPTION_H
#include <exception>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

class StrException : public std::exception
{
	std::string err;
public:
	StrException(std::string _err="") : err(_err) {}
	~StrException() throw() {}
	void SetString(const std::string& _err) { err = _err; }
	const std::string GetString() const { return err; }
};
#endif						  // PF_EXCEPTION_H
