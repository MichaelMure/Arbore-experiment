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

#include <string>
#include "packet_arg.h"
#include "net_proto.h"

const PacketArgType packet_args[NET_NB_MESSAGES][MAX_PACKET_ARGS] =
{
	// NET_NONE
	{ T_NONE },

	/* NET_HELLO */
	{
		/* NET_HELLO_NOW */     T_UINT32,
		/* NET_HELLO_FLAGS */   T_UINT32,
		/* NET_HELLO_PORT */    T_UINT32,
		/* NET_HELLO_VERSION */ T_STR,
		T_NONE
	},

	/* NET_MERGE */
	{
		T_NONE
	},

	/* NET_MKFILE */
	{
		/* NET_MKFILE_PATH */        T_STR,
		/* NET_MKFILE_MODE */        T_UINT32,
		/* NET_MKFILE_UID */         T_UINT32,
		/* NET_MKFILE_GID */         T_UINT32,
		/* NET_MKFILE_SIZE */        T_UINT64,
		/* NET_MKFILE_ACCESS_TIME */ T_UINT32,
		/* NET_MKFILE_CREATE_TIME */ T_UINT32,
		/* NET_MKFILE_MODIF_TIME */  T_UINT32,
		T_NONE
	},

	/* NET_RMFILE */
	{
		/* NET_RMFILE_PATH */ T_STR,
		T_NONE
	},

	// NET_PEER_CONNECTION
	{
		/* NET_PEER_LIST_ADDRESSES */ T_ADDRLIST,
		T_NONE
	},

	// NET_END_OF_MERGE
	{ T_NONE},

	// NET_END_OF_MERGE_ACK
	{ T_NONE},

};


