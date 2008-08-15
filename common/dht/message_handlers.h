/*
 * Copyright(C) 2008 Romain Bignon
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
 * This file contains some code from the Chimera's Distributed Hash Table,
 * written by CURRENT Lab, UCSB.
 *
 */

#ifndef _CHIMERA_MESSAGE_HANDLERS_H
#define _CHIMERA_MESSAGE_HANDLERS_H

#include "jobs/job.h"

class Message;

class IMessageHandler
{
public:

	virtual void operator() (const Message* message) = 0;
};

class JobHandleMessage : public Job
{
	IMessageHandler* handler;
	Message* message;

public:
	JobHandleMessage(IMessageHandler* _handler, Message* _message)
		: Job(0, REPEAT_NONE),
		   handler(_handler),
		   message(_message)
	{}

	bool Start();

	job_type GetType() const { return JOB_HANDLE_MESSAGE; }
	std::string GetName() const { return "JobHandleMessage"; }
};


#endif /* _CHIMERA_MESSAGE_HANDLERS_H */
