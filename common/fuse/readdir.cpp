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
#include <string.h>
/* At time, this headers are useless
#include <stdio.h>
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

int pf_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
off_t offset, struct fuse_file_info *fi)
{
	cache.Lock();
	DirEntry* dir = dynamic_cast<DirEntry*>(cache.Path2File(path));

	if(!dir)
	{
		cache.Unlock();
		return 1;
	}

	FileMap files = dir->GetFiles();
	for(FileMap::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		struct stat st;
		memset(&st, 0, sizeof st);
		/*st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;*/

		if(filler(buf, it->second->GetName().c_str(), &st, 0))
			break;
	}

	cache.Unlock();
	return 0;
}
