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
 */

#include "content_list.h"
#include "environment.h"

ContentList content_list;

const time_t remove_from_list_timeout = 15;

ContentList::~ContentList()
{
}

void ContentList::Loop()
{
	sleep(1);

	BlockLockMutex lock(this);
	iterator it = begin();
	while(it != end())
	{
		if(it->second.GetAccessTime() + remove_from_list_timeout < time(NULL))
		{
			pf_log[W_DEBUG] << "Remove \"" << it->first << "\" from the content_list";
			/* Force a sync to the disc */
			it->second.SyncToHdd(true);
			RemoveFile(it->first);
			break;
		}
		else
		{
			/* Sync what we can */
			it->second.SyncToHdd(false);
			++it;
		}
	}
}

void ContentList::OnStop()
{
	/* Force saving chunk to disk */
	for(iterator it = begin(); it != end(); ++it)
		it->second.SyncToHdd(true);
	sync();
}

FileContent& ContentList::GetFile(std::string path)
{
	BlockLockMutex lock(this);
	iterator it = find(path);
	if(it == end())
		insert(make_pair(path, FileContent(path)));

	it = find(path);
	return it->second;
}

FileContent& ContentList::GetFile(uint32_t ref)
{
	BlockLockMutex lock(this);
	std::string filename;
	std::map<uint32_t, std::string>::iterator it;

	//	TODO: handle this nicely
	if((it = my_refs.find(ref)) == my_refs.end())
		return GetFile("");

	return GetFile(it->second);
}

void ContentList::RemoveFile(std::string path)
{
	BlockLockMutex lock(this);

	iterator it = find(path);
	if(it == end())
		return;

	// Erase the content
	erase(it);

	// Find the ref and erase it
	uint32_t ref;
	std::map<uint32_t, std::string>::iterator ref_it;
	for(ref_it = my_refs.begin(); ref_it != my_refs.end(); ++ref_it)
	{
		if(ref_it->second == path)
		{
			ref = ref_it->first;
			my_refs.erase(ref_it);
			break;
		}
	}

	// Acknowledge peers we don't ref this file anymore
	std::map<std::string, IDList>::iterator referer_it;
	if((referer_it = refered_by.find(path)) != refered_by.end())
	{
		Packet packet(NET_UNREF_FILE);
		packet.SetArg(NET_UNREF_FILE_REF, ref);
		peers_list.SendMsg(referer_it->second, packet);
		refered_by.erase(referer_it);
	}
}

void ContentList::RemovePeerRefs(pf_id peer)
{
	BlockLockMutex lock(this);
	for(iterator it = begin(); it != end(); ++it)
		it->second.RemoveSharer(peer);
}

void ContentList::RefreshPeersRef(std::string path)
{
	BlockLockMutex lock(this);
	std::map<std::string, IDList>::iterator referers_list;

	if((referers_list = refered_by.find(path)) == refered_by.end())
		return;

	uint32_t ref = GetRef(path);
	FileContent& f = GetFile(path);
	off_t offset;
	off_t size;
	f.GetSharedContent(offset, size);

	Packet packet(NET_REFRESH_REF_FILE);
	packet.SetArg(NET_REFRESH_REF_FILE_REF, ref);
	packet.SetArg(NET_REFRESH_REF_FILE_OFFSET, (uint64_t)offset);
	packet.SetArg(NET_REFRESH_REF_FILE_SIZE, (uint64_t)size);
	peers_list.SendMsg(referers_list->second, packet);
}

uint32_t ContentList::GetRef(std::string filename)
{
	BlockLockMutex lock(this);
	std::map<uint32_t, std::string>::iterator it;

	/* TODO: optimize-me */
	for(it = my_refs.begin(); it != my_refs.end(); ++it)
	{
		if(it->second == filename)
			return it->first;
	}

	uint32_t ref = 0;
	while(my_refs.find(ref) != my_refs.end())
		ref++;
	my_refs[ref] = filename;
	pf_log[W_DEBUG] << "Giving ref " << ref << " to \"" << filename << "\"";

	return ref;
}

void ContentList::AddReferer(std::string path, pf_id referer)
{
	BlockLockMutex lock(this);
	if(refered_by.find(path) == refered_by.end())
		refered_by.insert(make_pair(path, IDList()));

	refered_by[path].insert(referer);
}

void ContentList::DelReferer(std::string path, pf_id referer)
{
	BlockLockMutex lock(this);
	std::map<std::string, IDList>::iterator ref_it;
	if((ref_it = refered_by.find(path)) == refered_by.end())
		return;

	IDList::iterator referer_it;
	if((referer_it = std::find(ref_it->second.begin(), ref_it->second.end(), referer)) != ref_it->second.end())
		ref_it->second.erase(referer_it);
}

void ContentList::DelReferer(pf_id referer)
{
	BlockLockMutex lock(this);
	std::map<std::string, IDList>::iterator ref;
	for(ref = refered_by.begin(); ref != refered_by.end(); ++ref)
	{
		IDList::iterator it;
		if((it = std::find(ref->second.begin(), ref->second.end(), referer)) != ref->second.end())
			ref->second.erase(it);
	}
}
void ContentList::SendRefFile(pf_id to, std::string filename)
{
	uint32_t ref = content_list.GetRef(filename);
	FileContent& f = content_list.GetFile(filename);
	off_t offset;
	off_t size;
	f.GetSharedContent(offset, size);

	Packet packet(NET_REF_FILE, environment.my_id.Get(), to);
	packet.SetArg(NET_REF_FILE_PATH, filename);
	packet.SetArg(NET_REF_FILE_REF, ref);
	packet.SetArg(NET_REF_FILE_OFFSET, (uint64_t)offset);
	packet.SetArg(NET_REF_FILE_SIZE, (uint64_t)size);
	peers_list.SendMsg(to, packet);

}
