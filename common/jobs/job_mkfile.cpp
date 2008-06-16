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

#include "job_mkfile.h"
#include "cache.h"
#include "pf_log.h"

bool JobMkFile::Start()
{
	try
	{
		cache.SetAttr(file, stat, sharers, sender, keep_newest, erase_on_modification);
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		try
		{
			cache.MkFile(file, stat, sharers, sender);
		}
		catch(Cache::NoSuchFileOrDir &e)
		{
			pf_log[W_DESYNCH] << "Unable to create " << file << ": File already exists";
			/* XXX: DO SOMETHING */
		}
	}
	catch(Cache::FileAlreadyExists &e)
	{
		pf_log[W_DESYNCH] << "Unable to create " << file << ": File already exists";
		/* XXX: DO SOMETHING */
	}

	return false;
}
