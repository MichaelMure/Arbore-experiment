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

#include <cstdlib>

#include <util/key.h>
#include <files/file_chunk.h>
#include <dht/data.h>

#include "pf_addr.h"
#include "addr_list.h"
#include "packet.h"
#include "packet_arg.h"
#include "packet_handler.h"
#include "packet_type_list.h"

#ifdef DEBUG
#define ASSERT assert
#else
#define ASSERT(x) if(!(x)) throw Malformated();
#endif

Packet::Packet(const PacketType& _type, const Key& _src, const Key& _dst)
			: type(_type),
			size(0),
			src(_src),
			dst(_dst),
			flags(_type.GetDefFlags()),
			seqnum(0),
			data(NULL)
{
}

Packet::Packet(const Packet& p)
			: type(p.type),
			size(p.size),
			src(p.src),
			dst(p.dst),
			flags(p.flags),
			seqnum(p.seqnum),
			data(NULL)
{
	if(size && p.data)
	{
		data = (char*) malloc(size);
		memcpy(data, p.data, size);
	}
	for(std::vector<PacketArgBase*>::const_iterator it = p.arg_lst.begin(); it != p.arg_lst.end(); ++it)
		arg_lst.push_back((*it)->clone());
}

Packet::Packet(char* header, size_t datasize)
			: type(0, NULL, 0, "NONE", T_END),
			size(0),
			data(NULL)
{
	uint32_t* p = (uint32_t*)header;
	uint32_t* s = p;

	/* Src key */
	while((size_t)(s - p) < Key::nlen) *s = ntohl(*s), s++;
	src = Key(p);
	p = s;

	/* Dst key */
	while(size_t(s - p) < Key::nlen) *s = ntohl(*s), s++;
	dst = Key(p);
	p = s;

	/* Type */
	uint32_t type_i = ntohl(*p++);

	try
	{
		type = packet_type_list.GetPacketType(type_i);
	}
	catch(PacketTypeList::UnknowType& e)
	{
		ASSERT(false);
	}

	/* Size */
	size = ntohl(*p++);

	/* Sequence number */
	seqnum = ntohl(*p++);

	/* Flags */
	flags = ntohl(*p++);

	ASSERT(datasize >= GetHeaderSize());
	ASSERT(size == datasize - GetHeaderSize());

	data = (char*) malloc(size);
	memcpy(data, p, size);

	BuildArgsFromData();
}

char* Packet::DumpBuffer()
{
	BuildDataFromArgs();

	char* dump = (char*) malloc(GetSize());
	uint32_t _type = htonl(type.GetType());
	uint32_t _size = htonl(size);
	uint32_t _seqnum = htonl(seqnum);
	uint32_t _flags = htonl(flags);
	char* ptr = dump;

	/* Src key */
	src.dump(ptr);
	ptr += Key::size;

 	/* Dst key */
	dst.dump(ptr);
	ptr += Key::size;

	/* Type */
	memcpy(ptr, &_type, sizeof(_type));
	ptr += sizeof _type;

	/* Size */
	memcpy(ptr, &_size, sizeof(_size));
	ptr += sizeof _size;

	/* Sequence number */
	memcpy(ptr, &_seqnum, sizeof(_seqnum));
	ptr += sizeof _seqnum;

	/* Flags */
	memcpy(ptr, &_flags, sizeof(_flags));
	ptr += sizeof _flags;

	/* Data */
	memcpy(ptr, data, GetDataSize());

	return dump;
}

uint32_t Packet::GetHeaderSize()
{
	return    Key::size                // src
	        + Key::size                // dst
	        + sizeof(uint32_t)         // type
	        + sizeof(uint32_t)         // size of data
	        + sizeof(uint32_t)         // sequence number
	        + sizeof(uint32_t);        // flags
}

uint32_t Packet::GetSize() const
{
	return size + GetHeaderSize();
}

Packet& Packet::operator=(const Packet& p)
{
	type = p.type;
	size = p.size;
	src = p.src;
	dst = p.dst;
	flags = p.flags;
	seqnum = p.seqnum;

	if(data)
	{
		free(data);
		data = NULL;
	}

	if(size && p.data)
	{
		data = (char*) malloc(size);
		memcpy(data, p.data, size);
	}

	for(std::vector<PacketArgBase*>::const_iterator it = p.arg_lst.begin(); it != p.arg_lst.end(); ++it)
		arg_lst.push_back((*it)->clone());

	return *this;
}

Packet::~Packet()
{
	if(data)
	{
		free(data);
		data = NULL;
	}

	/* DATA arg need to be freed, because they are pointer, unlike the others arg type. */
	for(PacketType::iterator it = type.begin(); it != type.end(); ++it)
		if(*it == T_DATA)
			delete GetArg<Data*>(it - type.begin());

	for(std::vector<PacketArgBase*>::iterator it = arg_lst.begin(); it != arg_lst.end(); ++it)
		delete *it;
}

void Packet::BuildArgsFromData()
{
	if(!data)
		return;
	char* p = data;
	for(PacketType::iterator it = type.begin(); it != type.end(); ++it)
	{
		size_t arg_no = it - type.begin();
		switch(*it)
		{
			case T_UINT32:
				SetArg(arg_no, Netutil::ReadInt32(p));
				p += sizeof(uint32_t);
				break;
			case T_UINT64:
				SetArg(arg_no, Netutil::ReadInt64(p));
				p += sizeof(uint64_t);
				break;
			case T_KEY:
				SetArg(arg_no, Key(p));
				p += Key::size;
				break;
			case T_STR:
				{
					std::string s = Netutil::ReadStr(p);
					p += Netutil::getSerialisedSize(s);
					SetArg(arg_no, s);
				}
				break;
			case T_ADDRLIST:
				{
					addr_list addl =  addr_list(p);
					p += addl.getSerialisedSize();
					SetArg(arg_no, addl);
				}
				break;
			case T_ADDR:
				SetArg(arg_no, pf_addr(p));
				p += pf_addr::size;
				break;
			case T_CHUNK:
				{
					FileChunk fc = FileChunk(p);
					p += fc.getSerialisedSize();
					SetArg(arg_no, fc);
				}
				break;
			case T_DATA:
				{
					Data* d = Data::createData(p);
					p += d->getSerialisedSize();
					SetArg(arg_no, d);
				}
				break;
			case T_END:
			default:
				ASSERT(false);
		}
	}

	if((uint32_t) (p-data) < size)
		pf_log[W_WARNING] << "There are some unread data in packet: " << *this;

	free(data);
	data = NULL;
}

void Packet::BuildDataFromArgs()
{
	if(data)
		return;

	size = 0;

	for(PacketType::iterator it = type.begin(); it != type.end(); ++it)
	{
		size_t arg_no = it - type.begin();
		switch(*it)
		{
			case T_UINT32:
				data = (char*) realloc(data, size+sizeof(uint32_t));
				Netutil::dump(GetArg<uint32_t>(arg_no), data+size);
				size += (uint32_t) sizeof(uint32_t);
				break;
			case T_UINT64:
				data = (char*) realloc(data, size+sizeof(uint64_t));
				Netutil::dump(GetArg<uint64_t>(arg_no), data+size);
				size += (uint32_t) sizeof(uint64_t);
				break;
			case T_KEY:
				data = (char*) realloc(data, size + Key::size);
				GetArg<Key>(arg_no).dump(data+size);
				size += (uint32_t) Key::size;
				break;
			case T_STR:
				{
					std::string s = GetArg<std::string>(arg_no);
					data = (char*) realloc(data, size + Netutil::getSerialisedSize(s));
					Netutil::dump(s, data+size);
					size += (uint32_t) Netutil::getSerialisedSize(s);
				}
				break;
			case T_ADDRLIST:
				{
					addr_list addl = GetArg<addr_list>(arg_no);
					data = (char*) realloc(data, size + addl.getSerialisedSize());
					addl.dump(data+size);
					size += (uint32_t) addl.getSerialisedSize();
				}
				break;
			case T_ADDR:
				data = (char*) realloc(data, size + pf_addr::size);
				GetArg<pf_addr>(arg_no).dump(data+size);
				size += (uint32_t) pf_addr::size;
				break;
			case T_CHUNK:
				{
					FileChunk fc = GetArg<FileChunk>(arg_no);
					data = (char*) realloc(data, size + fc.getSerialisedSize());
					fc.dump(data+size);
					size += (uint32_t) fc.getSerialisedSize();
				}
				break;
			case T_DATA:
				{
					Data* d = GetArg<Data*>(arg_no);
					data = (char*) realloc(data, size + d->getSerialisedSize());
					d->dump(data+size);
					size += (uint32_t) d->getSerialisedSize();
				}
				break;
			case T_END:
			default:
				ASSERT(false);
		}
	}
}

std::string Packet::GetStr() const
{
	std::string s, info;

	info = "[" + GetSrc().GetStr();
	info += "->" + GetDst().GetStr() + "] ";

	info += "<" + std::string(type.GetName());
	if(HasFlag(REQUESTACK))
		info += "(RACK)";
	if(HasFlag(ACK))
		info += "(ACK)";
	if(HasFlag(MUSTROUTE))
		info += "(ROUTE)";
	info += "> ";

	for(PacketType::const_iterator it = type.begin(); it != type.end(); ++it)
	{
		size_t arg_no = it - type.begin();
		if(s.empty() == false)
			s += ", ";
		switch(*it)
		{
			case T_UINT32: s += TypToStr(GetArg<uint32_t>(arg_no)); break;
			case T_UINT64: s += TypToStr(GetArg<uint64_t>(arg_no)); break;
			case T_KEY: s += GetArg<Key>(arg_no).GetStr(); break;
			case T_STR: s += "'" + GetArg<std::string>(arg_no) + "'"; break;
			case T_ADDRLIST:
			{
				addr_list v = GetArg<addr_list>(arg_no);
				std::string list;
				for(addr_list::const_iterator it2 = v.begin();
					it2 != v.end();
					++it2)
				{
					if(!list.empty()) list += ",";
					list += (*it2).GetStr();
				}
				s += "[" + list + "]";
				break;
			}
			case T_ADDR: s += GetArg<pf_addr>(arg_no).GetStr(); break;
			case T_CHUNK:
				s += "chunk(off:" + TypToStr(GetArg<FileChunk>(arg_no).GetOffset())
					+ " size:" +  TypToStr(GetArg<FileChunk>(arg_no).GetSize()) + ")";
				break;
			case T_DATA:
				s += "data(" + GetArg<Data*>(arg_no)->GetStr() + ")";
				break;
			case T_END:
			default:
				ASSERT(false);
		}
	}

	info += s;
	return info;
}
