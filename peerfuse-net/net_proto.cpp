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

#include <string>
#include "packet_arg.h"
#include "net_proto.h"

const char* type2str[NET_NB_MESSAGES] =
{
	/* NET_NONE */             NULL,
	/* NET_HELLO */            "HELLO",
	/* NET_MKFILE */           "MKFILE",
	/* NET_PEER_CONNECTION */  "PEER_CONNECTION",
	/* NET_END_OF_MERGE */     "END_OF_MERGE",
	/* NET_END_OF_MERGE_ACK */ "END_OF_MERGE_ACK",
	/* NET_PEER_BYE */         "PEER_BYE",
	/* NET_WANT_REF_FILE */    "WANT_REF_FILE",
	/* NET_REF_FILE */         "REF_FILE",
	/* NET_REFRESH_REF_FILE */ "REFRESH_REF_FILE",
	/* NET_WANT_CHUNK */       "WANT_CHUNK",
	/* NET_CHUNK */            "CHUNK",
	/* NET_LS_DIR */           "LS_DIR",
	/* NET_END_OF_LS */        "END_OF_LS",
};

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
		/* NET_MKFILE_SHARERS */         T_IDLIST,
		/* NET_MKFILE_PF_MODE */         T_UINT32,
		T_NONE
	},

	// NET_PEER_CONNECTION
	{
		/* NET_PEER_LIST_ADDRESSES */     T_ADDR,
		/* NET_PEER_LIST_NOW */           T_UINT32,
		/* NET_PEER_LIST_CERTIFICATE */   T_CERTIFICATE,
		T_NONE
	},

	// NET_END_OF_MERGE
	{ T_NONE},

	// NET_END_OF_MERGE_ACK
	{ T_NONE},

	// NET_PEER_BYE
	{ T_NONE},

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

	// NET_REFRESH_REF_FILE
	{
		/* NET_REFRESH_REF_FILE_REF */ T_UINT32, /* Reference of the file */
		/* NET_REFRESH_REF_FILE_OFFSET */ T_UINT64, /* Offset of the content we own */
		/* NET_REFRESH_REF_FILE_SIZE */ T_UINT64, /* Size of the content we own */
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
	},

	// NET_LS_DIR
	{
		/* NET_LS_DIR_PATH */ T_STR,
		T_NONE
	},

	// NET_END_OF_LS
	{
		/* NET_END_OF_LS_PATH */ T_STR,
		T_NONE
	}
};
