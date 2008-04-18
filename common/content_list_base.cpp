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
#include <time.h>
#include "content_list_base.h"
#include "log.h"

const time_t remove_from_list_timeout = 15;

ContentListBase::~ContentListBase()
{
}

void ContentListBase::Loop()
{
	sleep(1);

	BlockLockMutex lock(this);
	iterator it = begin();
	while(it != end())
	{
		if(it->second.GetAccessTime() + remove_from_list_timeout < time(NULL))
		{
			log[W_DEBUG] << "Remove \"" << it->first << "\" from the content_list";
			/* Force a sync to the disc */
			it->second.SyncToHdd(true);
			erase(it);
			break;
		}
		else
			++it;
	}
}

void ContentListBase::OnStop()
{
	/* Force saving chunk to disk */
	for(iterator it = begin(); it != end(); ++it)
		it->second.SyncToHdd(true);
}

FileContent& ContentListBase::GetFile(std::string path)
{
	BlockLockMutex lock(this);
	iterator it = find(path);
	if(it == end())
		insert(make_pair(path, FileContent(path)));

	it = find(path);
	return it->second;
}

void ContentListBase::RemoveFile(std::string path)
{
	BlockLockMutex lock(this);

	iterator it = find(path);
	if(it != end())
		erase(it);
}
