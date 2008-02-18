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

#ifndef PF_CERTIFICATE_H
#define PF_CERTIFICATE_H
#include <exception>

#include "public_key.h"
#include "private_key.h"

class Certificate
{
	int fd;

public:
	class BadCertificate : public std::exception {};
	class BadPassword : public std::exception {};

	Certificate();

	void LoadX509(std::string filename, std::string password);
	PrivateKey LoadPKCS12(std::string filename, std::string password);

	PublicKey GetPublicKey();

	const std::string GetCertificateInfos();
	const std::string GetIDFromCertificate();

	bool operator==(const Certificate&) { return true; }

};

#endif // PF_CERTIFICATE_H
