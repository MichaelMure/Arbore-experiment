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

const char* type2str[NET_NB_MESSAGES] =
{
	/* NET_NONE */                   NULL,
	/* NET_HELLO */                  "HELLO",
	/* NET_YOUR_ID */                "YOUR_ID",
	/* NET_START_MERGE */            "START_MERGE",
	/* NET_GET_STRUCT_DIFF */        "GET_STRUCT_DIFF",
	/* NET_MKFILE */                 "MKFILE",
	/* NET_RMFILE */                 "RMFILE",
	/* NET_FILE_SETATTR */           "FILE_SETATTR",
	/* NET_PEER_CONNECTION */        "PEER_CONNECTION",
	/* NET_PEER_CONNECTION_ACK */    "PEER_CONNECTION_ACK",
	/* NET_PEER_CONNECTION_RST */    "PEER_CONNECTION_RST",
	/* NET_PEER_CONNECTION_REJECTED */ "PEER_CONNECTION_REJECTED",
	/* NET_PEER_ALL_CONNECTED */     "PEER_ALL_CONNECTED",
	/* NET_END_OF_DIFF */            "END_OF_DIFF",
	/* NET_END_OF_MERGE */           "END_OF_MERGE",
	/* NET_END_OF_MERGE_ACK */       "END_OF_MERGE_ACK",
	/* NET_WHO_HAS_FILE */           "WHO_HAS_FILE",
	/* NET_I_HAVE_FILE */            "I_HAVE_FILE",
	/* NET_WANT_REF_FILE */          "WANT_REF_FILE",
	/* NET_REF_FILE */               "REF_FILE",
	/* NET_WANT_CHUNK */             "WANT_CHUNK",
	/* NET_CHUNK */                  "CHUNK",
};

const PacketArgType packet_args[NET_NB_MESSAGES][MAX_PACKET_ARGS] =
{
	// NET_NONE
	{ T_NONE },

	/* NET_HELLO */
	{
		/* NET_HELLO_NOW */     T_UINT32,
		/* NET_HELLO_PORT */    T_UINT32,
		/* NET_HELLO_VERSION */ T_STR,
		/* NET_HELLO_MY_ID */   T_UINT32,
		T_NONE
	},

	/* NET_YOUR_ID */
	{
		/* NET_YOUR_ID_ID */    T_UINT32,
		T_NONE
	},

	/* NET_MERGE */
	{
		T_NONE
	},

	/* NET_GET_STRUCT_DIFF */
	{
		/* NET_GET_STRUCT_DIFF_LAST_CONNECTION */ T_UINT32,
		T_NONE
	},

	/* NET_MKFILE */
	{
		/* NET_MKFILE_PATH */            T_STR,
		/* NET_MKFILE_MODE */            T_UINT32,
		/* NET_MKFILE_UID */             T_UINT32,
		/* NET_MKFILE_GID */             T_UINT32,
		/* NET_MKFILE_SIZE */            T_UINT64,
		/* NET_MKFILE_ACCESS_TIME */     T_UINT32,
		/* NET_MKFILE_CREATE_TIME */     T_UINT32,
		/* NET_MKFILE_MODIF_TIME */      T_UINT32,
		/* NET_MKFILE_META_MODIF_TIME */ T_UINT32,
		T_NONE
	},

	/* NET_RMFILE */
	{
		/* NET_RMFILE_PATH */ T_STR,
		T_NONE
	},

	/* NET_FILE_SETATTR */
	{
		/* NET_MKFILE_PATH */            T_STR,
		/* NET_MKFILE_MODE */            T_UINT32,
		/* NET_MKFILE_UID */             T_UINT32,
		/* NET_MKFILE_GID */             T_UINT32,
		/* NET_MKFILE_SIZE */            T_UINT64,
		/* NET_MKFILE_ACCESS_TIME */     T_UINT32,
		/* NET_MKFILE_CREATE_TIME */     T_UINT32,
		/* NET_MKFILE_MODIF_TIME */      T_UINT32,
		/* NET_MKFILE_META_MODIF_TIME */ T_UINT32,
		T_NONE
	},

	// NET_PEER_CONNECTION
	{
		/* NET_PEER_CONNECTION_ADDRESS */ T_ADDR,
		T_NONE
	},

	// NET_PEER_CONNECTION_ACK
	{
		/* NET_PEER_CONNECTION_ACK_ADDRESS */ T_ADDR,
		T_NONE
	},

	// NET_PEER_CONNECTION_RST
	{
		/* NET_PEER_CONNECTION_RST_ADDRESS */ T_ADDR,
		T_NONE
	},

	// NET_PEER_CONNECTION_REJECTED
	{
		/* NET_PEER_CONNECTION_REJECTED_ADDRESS */ T_ADDR,
		T_NONE
	},

	// NET_PEER_ALL_CONNECTED
	{ T_NONE },

	// NET_END_OF_DIFF
	{ T_NONE },

	// NET_END_OF_MERGE
	{ T_NONE },

	// NET_END_OF_MERGE_ACK
	{ T_NONE },

	// NET_WHO_HAS_FILE
	{
		/* NET_WHO_HAS_FILE_PATH */ T_STR,
		T_NONE
	},

	// NET_I_HAVE_FILE
	{
		/* NET_I_HAVE_FILE_PATH */ T_STR,
		T_NONE
	},

	// NET_WANT_REF_FILE
	{
		/* NET_WANT_REF_FILE_PATH */ T_STR,
		T_NONE
	},

	// NET_REF_FILE
	{
		/* NET_REF_FILE_PATH */ T_STR,
		/* NET_REF_FILE_REF */ T_UINT32,
		/* NET_REF_FILE_OFFSET */ T_UINT64,
		/* NET_REF_FILE_SIZE */ T_UINT64,
		T_NONE
	},

	// NET_WANT_CHUNK
	{
		/* NET_WANT_CHUNK_REF */ T_UINT32,
		/* NET_WANT_CHUNK_OFFSET */ T_UINT64,
		/* NET_WANT_CHUNK_SIZE */ T_UINT32,
		T_NONE
	},

	// NET_CHUNK
	{
		/* NET_CHUNK_REF */ T_UINT32,
		/* NET_CHUNK_CHUNK */ T_CHUNK,
		T_NONE
	}
};
