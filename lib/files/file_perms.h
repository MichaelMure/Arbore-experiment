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
 *
 */

#ifndef FILE_PERMS_H
#define FILE_PERMS_H

#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>

/** This class represent a file permission */
class FilePermissions
{
public:
	/* Special peerfuse flags. */
	enum
	{
		S_PF_REMOVED = 1 << 0
	};

	uint32_t pf_mode; /** Special peerfuse flag holder */
	mode_t  mode; /** Unix file mode (e.g. 0755 ) */
	uid_t uid; /** User ID of file */
	gid_t gid; /** Group ID of file */

	FilePermissions();
};
#endif						  /* FILE_PERMS_H */
