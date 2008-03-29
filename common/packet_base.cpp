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

#include <assert.h>
#include <stdio.h>
#include <netinet/in.h>				  // htonl, ntohl
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include "packet_base.h"
#include "packet_arg.h"
#include "pf_types.h"
#include "net_proto.h"
#include "tools.h"

#ifdef DEBUG
#define ASSERT assert
#else
#define ASSERT(x) if(!(x)) throw Malformated();
#endif

PacketBase::PacketBase(msg_type _type)
			: type(_type),
			size(0),
			datas(NULL)
{
}

PacketBase::PacketBase(const PacketBase& p)
			: type(p.type),
			size(p.size),
			datas(NULL)
{
	if(size)
	{
		datas = new char [size];
		memcpy(datas, p.datas, size);
	}
	for(std::vector<PacketArgBase*>::const_iterator it = p.arg_lst.begin(); it != p.arg_lst.end(); ++it)
		arg_lst.push_back((*it)->clone());
}

PacketBase& PacketBase::operator=(const PacketBase& p)
{
	type = p.type;
	size = p.size;

	if(size)
	{
		datas = new char [size];
		memcpy(datas, p.datas, size);
	}
	else
		datas = NULL;

	for(std::vector<PacketArgBase*>::const_iterator it = p.arg_lst.begin(); it != p.arg_lst.end(); ++it)
		arg_lst.push_back((*it)->clone());

	return *this;
}

PacketBase::~PacketBase()
{
	if(datas)
		delete []datas;

	for(std::vector<PacketArgBase*>::iterator it = arg_lst.begin(); it != arg_lst.end(); ++it)
		delete *it;
}

bool PacketBase::ReceiveContent(Connection* conn) throw(Malformated)
{
	char* buf;
	if(!conn->Read(&buf, GetDataSize()))
		return false;

	memcpy(datas, buf, GetDataSize());
	delete []buf;

	BuildArgsFromData();
	return true;
}

uint32_t PacketBase::GetHeaderSize()
{
	#if defined(PF_NET)
	return sizeof(pf_id)			  // id_src
		+ sizeof(pf_id)			  // id_dst
		+ sizeof(uint32_t)		  // size of the packet
		+ sizeof(uint32_t);		  // size of the type
	#elif defined(PF_LAN)
	return sizeof(uint32_t)			  // size of the packet
		+ sizeof(uint32_t);		  // size of the type
	#else
	#error "Hu ?"
	#endif
}

uint32_t PacketBase::GetSize() const
{
	return size + GetHeaderSize();
}

PacketBase& PacketBase::Write(uint32_t nbr)
{
	char* new_datas = new char [size + sizeof nbr];

	nbr = htonl(nbr);
	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;
	memcpy(new_datas + size, &nbr, sizeof(nbr));
	size += sizeof(nbr);
	datas = new_datas;

	return *this;
}

PacketBase& PacketBase::Write(uint64_t nbr)
{
	char* new_datas = new char [size + sizeof nbr];

	nbr = htonll(nbr);
	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;
	memcpy(new_datas + size, &nbr, sizeof(nbr));
	size += sizeof(nbr);
	datas = new_datas;

	return *this;
}

PacketBase& PacketBase::Write(pf_addr addr)
{
	char* new_datas = new char [size + sizeof addr];

	addr = pf_addr_ton(addr);
	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;
	memcpy(new_datas + size, &addr, sizeof(addr));
	size += sizeof(addr);
	datas = new_datas;

	return *this;
}

PacketBase& PacketBase::Write(std::string str)
{
	uint32_t str_len = str.size();
	Write(str_len);
	char* new_datas = new char [size + str_len];
	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;

	memcpy(new_datas + size, str.c_str(), str_len);
	size += str_len;
	datas = new_datas;

	return *this;
}

PacketBase& PacketBase::Write(AddrList addr_list)
{
	Write((uint32_t)addr_list.size());

	char* new_datas = new char [size + (addr_list.size() * sizeof(pf_addr))];

	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;

	char* ptr = new_datas + size;

	for(AddrList::iterator it = addr_list.begin(); it != addr_list.end(); ++it)
	{
		pf_addr addr = pf_addr_ton(*it);
		memcpy(ptr, &addr, sizeof(pf_addr));
	}
	size += addr_list.size() * sizeof(pf_addr);
	datas = new_datas;

	return *this;
}

void PacketBase::Send(Connection* conn)
{
	BuildDataFromArgs();
	char* buf = DumpBuffer();
	conn->Write(buf, GetSize());
	delete []buf;
}

uint32_t PacketBase::ReadInt32()
{
	ASSERT(size >= sizeof(uint32_t));
	uint32_t val = ntohl(*(uint32_t*)datas);

	char* new_datas;
	size -= sizeof(uint32_t);
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + sizeof(uint32_t), size);
	}

	delete []datas;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return val;
}

uint64_t PacketBase::ReadInt64()
{
	ASSERT(size >= sizeof(uint64_t));
	uint64_t val = ntohll(*(uint64_t*)datas);

	char* new_datas;
	size -= sizeof(uint64_t);
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + sizeof(uint64_t), size);
	}

	delete []datas;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return val;
}

pf_addr PacketBase::ReadAddr()
{
	ASSERT(size >= sizeof(pf_addr));
	pf_addr val = nto_pf_addr(*(pf_addr*)datas);

	char* new_datas;
	size -= sizeof(pf_addr);
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + sizeof(pf_addr), size);
	}

	delete []datas;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return val;
}

std::string PacketBase::ReadStr()
{
	uint32_t str_size = ReadInt32();
	ASSERT(size >= str_size);
	char* str = new char [str_size+1];

	memcpy(str, datas, str_size);
	str[str_size] = '\0';
	std::string val = std::string(str);

	char* new_datas;
	size -= str_size;
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + str_size, size);
	}

	delete []datas;
	delete []str;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return val;
}

AddrList PacketBase::ReadAddrList()
{
	AddrList addr_list;
	uint32_t list_size = ReadInt32();
	ASSERT((size * sizeof(pf_addr)) >= list_size);

	for(uint32_t i = 0; i < list_size; ++i)
	{
		pf_addr addr;
		memcpy(&addr, datas, sizeof(pf_addr));
		addr_list.push_back(nto_pf_addr(addr));
	}

	char* new_datas;
	size -= list_size * sizeof(pf_addr);
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + (list_size * sizeof(pf_addr)), size);
	}

	delete []datas;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return addr_list;
}

void PacketBase::BuildArgsFromData()
{
	for(size_t arg_no = 0; packet_args[type][arg_no] != T_NONE; ++arg_no)
		switch(packet_args[type][arg_no])
		{
			case T_UINT32: SetArg(arg_no, ReadInt32()); break;
			case T_UINT64: SetArg(arg_no, ReadInt64()); break;
			case T_STR: SetArg(arg_no, ReadStr()); break;
			case T_ADDRLIST: SetArg(arg_no, ReadAddrList()); break;
			case T_ADDR: SetArg(arg_no, ReadAddr()); break;
			default: throw Malformated();
		}
}

void PacketBase::BuildDataFromArgs()
{
	for(size_t arg_no = 0; packet_args[type][arg_no] != T_NONE; ++arg_no)
		switch(packet_args[type][arg_no])
		{
			case T_UINT32: Write(GetArg<uint32_t>(arg_no)); break;
			case T_UINT64: Write(GetArg<uint64_t>(arg_no)); break;
			case T_STR: Write(GetArg<std::string>(arg_no)); break;
			case T_ADDRLIST: Write(GetArg<AddrList>(arg_no)); break;
			case T_ADDR: Write(GetArg<pf_addr>(arg_no)); break;
			default: throw Malformated();
		}
}

std::string PacketBase::GetPacketInfo() const
{
	std::string s, info;

	info = "<" + std::string(type2str[GetType()]) + "> ";

	for(size_t arg_no = 0; packet_args[type][arg_no] != T_NONE; ++arg_no)
	{
		if(s.empty() == false)
			s += ", ";
		switch(packet_args[type][arg_no])
		{
			case T_UINT32: s += TypToStr(GetArg<uint32_t>(arg_no)); break;
			case T_UINT64: s += TypToStr(GetArg<uint64_t>(arg_no)); break;
			case T_STR: s += "'" + GetArg<std::string>(arg_no) + "'"; break;
			case T_ADDRLIST:
			{
				std::vector<pf_addr> v = GetArg<std::vector<pf_addr> >(arg_no);
				std::string list;
				for(std::vector<pf_addr>::const_iterator it = v.begin();
					it != v.end();
					++it)
				{
					if(!list.empty()) list += ",";
					list += pf_addr2string(*it);
				}
				s += "[" + list + "]";
				break;
			}
			case T_ADDR: s += pf_addr2string(GetArg<pf_addr>(arg_no)); break;
			default: throw Malformated();
		}
	}

	info += s;
	return info;
}
