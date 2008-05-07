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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
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

int pf_mkdir(const char *path, mode_t mode)
{
	try
	{
		/* it appears there isn't S_IFDIR flag in 'mode',
		 * but we know that this is a dir, and MkFile
		 * will create a DirEntry only if this flag is set.
		 */
		pf_stat stat;
		stat.mode = mode|S_IFDIR;
		stat.ctime = time(NULL);
		stat.atime = stat.ctime;
		stat.mtime = stat.ctime;
		stat.meta_mtime = stat.ctime;

		cache.MkFile(path, stat, IDList());
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		return -ENOENT;			  /* No such file or directory */
	}
	catch(Cache::FileAlreadyExists &e)
	{
		return -EEXIST;
	}

	return 0;
}
