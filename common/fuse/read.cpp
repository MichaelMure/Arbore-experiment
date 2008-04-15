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

#define FUSE_USE_VERSION 26
#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include "cache.h"
#include "log.h"

int pf_read(const char *path, char *buf, size_t size,
			off_t offset, struct fuse_file_info *fi)
{
	try
	{
		int bytes_read = cache.Read(std::string(path), buf, size, offset);
		return bytes_read;
	}
	catch(...)
	{
		log[W_ERR] << "Failed to write to " << path;
		return -ENOENT;
	}
	return 0;
}

