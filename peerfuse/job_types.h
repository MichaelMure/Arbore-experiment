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

#ifndef JOB_TYPES_H
#define JOB_TYPES_H

enum job_type
{
	/* MKfile triggered by a peer */
	JOB_MKFILE,
	/* SetAttr triggered by a peer */
	JOB_FILE_SETATTR,
	/* A peer for a reference to a file */
	JOB_SEND_REF_FILE,
	/* A peer told us the part of the file he has */
	JOB_SET_SHARED_PART,
	/* Send a chunk to a peer */
	JOB_SEND_CHUNK,
	/* Receive a chunk from a peer */
	JOB_SET_CHUNK,
	/* Tries connecting to a peer */
	JOB_NEW_CONNECT,
	/* Tries connecting to a peer because an other
	   peer asked us to it */
	JOB_NEW_CONN_REQ,
	/* Tries connecting to a list of peers */
	JOB_NEW_CONN_QUEUE,
	/* Update responsibles files */
	JOB_UPDATE_RESP_FILES,
	/* Send list of files in a directory to a peer */
	JOB_LS_DIR,
	/* Tell cache we receive all of the directory files */
	JOB_END_OF_LS,
	/* Delay a send of a NET_MKFILE message */
	JOB_SEND_MKFILE,
	/* Used to handle a received message */
	JOB_HANDLE_MESSAGE,
};
#endif