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

#ifndef JOB_TYPES_H
#define JOB_TYPES_H

enum job_type
{
	/* MKfile triggered by a peer */
	JOB_MKFILE,
	/* Rmfile triggered by a peer */
	JOB_RMFILE,
	/* SetAttr triggered by a peer */
	JOB_FILE_SETATTR,
	/* Notify the cache and the network about filesize modification */
	JOB_CHANGE_FILESIZE,
	/* Tries connecting to a peer */
	JOB_NEW_CONNECT,
	/* Tries connecting to a peer because an other
	   peer asked us to it */
	JOB_NEW_CONN_REQ,
	/* Tries connecting to a list of peers */
	JOB_NEW_CONN_QUEUE,
	/* Make the peer send all packet he has in its sending queue */
	JOB_FLUSH_PEER,
	/* Update responsibles files */
	JOB_UPDATE_RESP_FILES,
};
#endif
