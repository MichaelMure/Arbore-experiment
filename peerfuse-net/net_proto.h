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

#ifndef NET_PROTO_H
#define NET_PROTO_H

/* Please see the PROTOCOL file */

enum msg_type
{
	NET_NONE = 0,
	NET_HELLO,
	NET_START_MERGE,
	NET_MKFILE,
	NET_RMFILE,
	NET_PEER_CONNECTION,
	NET_END_OF_MERGE,
	NET_END_OF_MERGE_ACK,

	NET_NB_MESSAGES
};

enum net_hello_args
{
	NET_HELLO_NOW,				  // T_UINT32
	#define NET_HELLO_FLAGS_HIGHLINK 0x01
	NET_HELLO_FLAGS,			  // T_UINT32
	NET_HELLO_PORT,				  // T_UINT32
	NET_HELLO_VERSION,			  // T_STR
	NET_HELLO_ARG_SIZE
};

enum net_peer_connection
{
	NET_PEER_CONNECTION_ADDRESS,		  /* T_ADDR */
	NET_PEER_CONNECTION_CERTIFICATE,	  /* T_STR */
};

// NET_MKFILE
enum net_mkfile_args
{
	NET_MKFILE_PATH,
	NET_MKFILE_MODE,			  // file, dir, symlink ... read/write permissions
	NET_MKFILE_UID,
	NET_MKFILE_GID,
	NET_MKFILE_SIZE,
	NET_MKFILE_ACCESS_TIME,
	NET_MKFILE_CREATE_TIME,
	NET_MKFILE_MODIF_TIME,
	NET_MKFILE_META_MODIF_TIME,
	// ...
};

// NET_RMFILE
enum net_rmfile
{
	NET_RMFILE_PATH
	// ...
};
#endif						  /* NET_PROTO_H */
