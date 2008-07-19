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

#include "job_ls_dir.h"
#include "cache.h"
#include "environment.h"
#include "peers_list.h"

bool JobLsDir::Start()
{
	try
	{
		cache.SendDirFiles(path, id);
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		/* File doesn't exist. We send NET_END_OF_LS message... */
		Packet pckt = Packet(NET_END_OF_LS, environment.my_id.Get(), id);
		pckt.SetArg(NET_END_OF_LS_PATH, path);
		peers_list.SendMsg(id, pckt);
	}
	return false;
}
