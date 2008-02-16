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
#include <errno.h>
/* At time, this headers are useless
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
*/

#include "pf_fuse.h"
#include "cache.h"

int pf_unlink(const char *path)
{
	try
	{
		cache.RmFile(path, M_PROPAGATE);
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		return -ENOENT; /* No such file or directory */
	}
	catch(Cache::DirNotEmpty &e)
	{
		return -ENOTEMPTY;
	}

	return 0;
}
