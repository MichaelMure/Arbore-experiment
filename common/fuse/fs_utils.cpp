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

#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include "fs_utils.h"

bool check_filename(std::string f)
{
	if(f.find("/../") != std::string::npos)
		return false;
	if(f.find("../") == 0)
		return false;
	return true;
}

bool check_filemode(mode_t m)
{
	mode_t authorized = S_IFREG | S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
	return !((~authorized) & m);
}
