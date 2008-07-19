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

#include "job_advertise_file.h"
#include "content_list.h"
#include "packet.h"
#include "peers_list.h"
#include "net_proto.h"

bool JobAdvertiseFile::Start()
{
	if(content_list.HaveFile(filename))
	{
		Packet packet(NET_I_HAVE_FILE);
		packet.SetArg(NET_I_HAVE_FILE_PATH, filename);
		peers_list.SendMsg(advertise_to, packet);
	}
	return false;
}
