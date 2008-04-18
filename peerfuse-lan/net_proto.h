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
	NET_YOUR_ID,
	NET_START_MERGE,
	NET_GET_STRUCT_DIFF,
	NET_MKFILE,
	NET_RMFILE,
	NET_FILE_SETATTR,
	NET_PEER_CONNECTION,
	NET_PEER_CONNECTION_ACK,
	NET_PEER_CONNECTION_RST,
	NET_PEER_CONNECTION_REJECTED,
	NET_PEER_ALL_CONNECTED,
	NET_END_OF_DIFF,
	NET_END_OF_MERGE,
	NET_END_OF_MERGE_ACK,

	NET_WHO_HAS_FILE,
	NET_I_HAVE_FILE,
	NET_WANT_REF_FILE,
	NET_REF_FILE,
	NET_WANT_CHUNK,
	NET_CHUNK,

	NET_NB_MESSAGES
};

enum net_hello_args
{
	NET_HELLO_NOW,				  // T_INT
	NET_HELLO_PORT,				  // T_UINT32
	NET_HELLO_VERSION,			  // T_STR
	NET_HELLO_MY_ID,			  // T_UINT32
	NET_HELLO_ARG_SIZE
};

enum net_your_id_args
{
	NET_YOUR_ID_ID,				  // T_UINT32
	NET_YOUR_ID_ARG_SIZE
};

enum net_get_struct_diff_args
{
	NET_GET_STRUCT_DIFF_LAST_CONNECTION,	  // T_INT
	NET_GET_STRUCT_DIFF_ARG_SIZE
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

// NET_FILE_SETATTR
enum net_file_setattr
{
	NET_FILE_SETATTR_PATH,
	NET_FILE_SETATTR_MODE,			  // file, dir, symlink ... read/write permissions
	NET_FILE_SETATTR_UID,
	NET_FILE_SETATTR_GID,
	NET_FILE_SETATTR_SIZE,
	NET_FILE_SETATTR_ACCESS_TIME,
	NET_FILE_SETATTR_CREATE_TIME,
	NET_FILE_SETATTR_MODIF_TIME,
	NET_FILE_SETATTR_META_MODIF_TIME,
};

enum net_peer_connection
{
	NET_PEER_CONNECTION_ADDRESS,
};

enum net_peer_connection_ack
{
	NET_PEER_CONNECTION_ACK_ADDRESS,
};

enum net_peer_connection_rst
{
	NET_PEER_CONNECTION_RST_ADDRESS,
};

enum net_peer_connection_rejected
{
	NET_PEER_CONNECTION_REJECTED_ADDRESS,
};

enum net_peer_all_connected
{
};

enum net_who_has_file
{
	NET_WHO_HAS_FILE_PATH,
};

enum net_i_have_file
{
	NET_I_HAVE_FILE_PATH
};

enum net_want_ref_file
{
	NET_WANT_REF_FILE_PATH
};

enum net_ref_file_path
{
	NET_REF_FILE_PATH,
	NET_REF_FILE_REF,
	NET_REF_FILE_OFFSET,
	NET_REF_FILE_SIZE
};

enum net_want_chunk
{
	NET_WANT_CHUNK_REF,
	NET_WANT_CHUNK_OFFSET,
	NET_WANT_CHUNK_SIZE
};

enum net_chunk
{
	NET_CHUNK_REF,
	NET_CHUNK_CHUNK,
};

extern const char* type2str[NET_NB_MESSAGES];
#endif						  /* NET_PROTO_H */
