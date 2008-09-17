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

#include "packet.h"
#include "packet_arg.h"
#include "packet_type_list.h"
#include "packet_handler.h"
#include "dht/key.h"

#ifdef DEBUG
#define ASSERT assert
#else
#define ASSERT(x) if(!(x)) throw Malformated();
#endif

Packet::Packet(PacketType _type, const Key& _src, const Key& _dst)
			: type(_type),
			size(0),
			src(_src),
			dst(_dst),
			flags(0),
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
	if(size)
	{
		data = new char [size];
		memcpy(data, p.data, size);
	}
	for(std::vector<PacketArgBase*>::const_iterator it = p.arg_lst.begin(); it != p.arg_lst.end(); ++it)
		arg_lst.push_back((*it)->clone());
}

Packet::Packet(PacketTypeList* pckt_type_list, char* header, size_t datasize)
			: type(0, NULL, "NONE", T_END),
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

	ASSERT(type_i < pckt_type_list->size());

	type = pckt_type_list->GetPacketType(type_i);

	/* Size */
	size = ntohl(*p++);

	/* Sequence number */
	seqnum = ntohl(*p++);

	/* Flags */
	flags = ntohl(*p++);

	ASSERT(datasize >= GetHeaderSize());

	SetContent((char*)p, datasize - GetHeaderSize());
}

char* Packet::DumpBuffer()
{
	BuildDataFromArgs();

	pf_log[W_DEBUG] << GetSize();
	char* dump = new char [GetSize()];
	uint32_t _type = htonl(type.GetType());
	uint32_t _size = htonl(size);
	uint32_t _seqnum = htonl(seqnum);
	uint32_t _flags = htonl(flags);
	char* ptr = dump;

	/* Src key */
	const uint32_t* key = src.GetArray();
	for(size_t i = 0; i < Key::nlen; ++i)
	{
		memcpy(ptr, key+i, sizeof(key[i]));
		ptr += sizeof(key[i]);
	}
	/* Dst key */
	key = dst.GetArray();
	for(size_t i = 0; i < Key::nlen; ++i)
	{
		memcpy(ptr, key+i, sizeof(key[i]));
		ptr += sizeof(key[i]);
	}

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

	pf_log[W_DEBUG] << ptr - dump + GetDataSize();

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

	if(data)
		delete []data;

	if(size)
	{
		data = new char [size];
		memcpy(data, p.data, size);
	}
	else
		data = NULL;

	for(std::vector<PacketArgBase*>::const_iterator it = p.arg_lst.begin(); it != p.arg_lst.end(); ++it)
		arg_lst.push_back((*it)->clone());

	return *this;
}

Packet::~Packet()
{
	if(data)
		delete []data;

	for(std::vector<PacketArgBase*>::iterator it = arg_lst.begin(); it != arg_lst.end(); ++it)
		delete *it;
}

void Packet::SetContent(const char* buf, size_t _size)
{
	ASSERT(GetDataSize() == _size);

	data = new char [GetDataSize()];
	memcpy(data, buf, GetDataSize());

	BuildArgsFromData();
}

void Packet::Handle(PacketTypeList& pckt_type_list, const Host& sender) const
{
	(*type.GetHandler()) (pckt_type_list, sender, *this);
}

void Packet::BuildArgsFromData()
{
	if(!data)
		return;
	for(PacketType::iterator it = type.begin(); it != type.end(); ++it)
	{
		size_t arg_no = it - type.begin();
		switch(*it)
		{
			case T_UINT32: SetArg(arg_no, ReadInt32()); break;
			case T_UINT64: SetArg(arg_no, ReadInt64()); break;
			case T_KEY: SetArg(arg_no, ReadKey()); break;
			case T_STR: SetArg(arg_no, ReadStr()); break;
			case T_ADDRLIST: SetArg(arg_no, ReadAddrList()); break;
			case T_ADDR: SetArg(arg_no, ReadAddr()); break;
			case T_CHUNK: SetArg(arg_no, ReadChunk()); break;
			default: throw Malformated();
		}
	}
	if(data)
		pf_log[W_WARNING] << "There are some unread data in packet: " << GetPacketInfo();
}

void Packet::BuildDataFromArgs()
{
	if(data)
		return;
	for(PacketType::iterator it = type.begin(); it != type.end(); ++it)
	{
		size_t arg_no = it - type.begin();
		switch(*it)
		{
			case T_UINT32: Write(GetArg<uint32_t>(arg_no)); break;
			case T_UINT64: Write(GetArg<uint64_t>(arg_no)); break;
			case T_KEY: Write(GetArg<Key>(arg_no)); break;
			case T_STR: Write(GetArg<std::string>(arg_no)); break;
			case T_ADDRLIST: Write(GetArg<AddrList>(arg_no)); break;
			case T_ADDR: Write(GetArg<pf_addr>(arg_no)); break;
			case T_CHUNK: Write(GetArg<FileChunk>(arg_no)); break;
			default: throw Malformated();
		}
	}
}

std::string Packet::GetPacketInfo() const
{
	std::string s, info;

	info = "[" + GetSrc().str();
	info += "->" + GetDst().str() + "] ";

	info = "<" + std::string(type.GetName()) + "> ";

	for(PacketType::const_iterator it = type.begin(); it != type.end(); ++it)
	{
		size_t arg_no = it - type.begin();
		if(s.empty() == false)
			s += ", ";
		switch(*it)
		{
			case T_UINT32: s += TypToStr(GetArg<uint32_t>(arg_no)); break;
			case T_UINT64: s += TypToStr(GetArg<uint64_t>(arg_no)); break;
			case T_KEY: s += GetArg<Key>(arg_no).str(); break;
			case T_STR: s += "'" + GetArg<std::string>(arg_no) + "'"; break;
			case T_ADDRLIST:
			{
				AddrList v = GetArg<AddrList>(arg_no);
				std::string list;
				for(AddrList::const_iterator it = v.begin();
					it != v.end();
					++it)
				{
					if(!list.empty()) list += ",";
					list += (*it).str();
				}
				s += "[" + list + "]";
				break;
			}
			case T_ADDR: s += GetArg<pf_addr>(arg_no).str(); break;
			case T_CHUNK:
				s += "chunk(off:" + TypToStr(GetArg<FileChunk>(arg_no).GetOffset())
					+ " size:" +  TypToStr(GetArg<FileChunk>(arg_no).GetSize()) + ")";
				break;
			default: throw Malformated();
		}
	}

	info += s;
	return info;
}

/* T_UINT32 */
uint32_t Packet::ReadInt32()
{
	ASSERT(size >= sizeof(uint32_t));
	uint32_t val = ntohl(*(uint32_t*)data);

	char* new_data = NULL;
	size -= (uint32_t)sizeof(uint32_t);
	if(size > 0)
	{
		new_data = new char [size];
		memcpy(new_data, data + sizeof(uint32_t), size);
	}

	delete []data;

	if(size > 0)
		data = new_data;
	else
		data = NULL;

	return val;
}


Packet& Packet::Write(uint32_t nbr)
{
	ASSERT(((uint32_t)sizeof(nbr)) + size >= size);

	char* new_data = new char [size + sizeof nbr];

	nbr = htonl(nbr);
	if(data)
		memcpy(new_data, data, size);
	if(data)
		delete []data;
	memcpy(new_data + size, &nbr, sizeof(nbr));
	size += (uint32_t)sizeof(nbr);
	data = new_data;

	return *this;
}

/* T_UINT64 */
uint64_t Packet::ReadInt64()
{
	ASSERT(size >= sizeof(uint64_t));
	uint64_t val = ntohll(*(uint64_t*)data);

	char* new_data;
	size -= (uint32_t)sizeof(uint64_t);
	if(size > 0)
	{
		new_data = new char [size];
		memcpy(new_data, data + sizeof(uint64_t), size);
	}

	delete []data;

	if(size > 0)
		data = new_data;
	else
		data = NULL;

	return val;
}

Packet& Packet::Write(uint64_t nbr)
{
	ASSERT(((uint32_t)sizeof(nbr)) + size >= size);

	char* new_data = new char [size + sizeof nbr];

	nbr = htonll(nbr);
	if(data)
		memcpy(new_data, data, size);
	if(data)
		delete []data;
	memcpy(new_data + size, &nbr, sizeof(nbr));
	size += (uint32_t)sizeof(nbr);
	data = new_data;

	return *this;
}

/* T_KEY */
Key Packet::ReadKey()
{
	ASSERT(size >= Key::size);

	uint32_t val[Key::nlen];

	for(size_t i = 0; i < Key::nlen; ++i)
		val[i] = ntohl(((uint32_t*)data)[i]);

	char* new_data;
	Key key(val);

	size -= (uint32_t)Key::size;
	if(size > 0)
	{
		new_data = new char [size];
		memcpy(new_data, data + Key::size, size);
	}

	delete []data;

	if(size > 0)
		data = new_data;
	else
		data = NULL;

	return key;
}

Packet& Packet::Write(Key key)
{
	ASSERT(((uint32_t)Key::size) + size >= size);

	char* new_data = new char [size + Key::size];

	if(data)
		memcpy(new_data, data, size);
	if(data)
		delete []data;

	const uint32_t* val = key.GetArray();
	for(size_t i = 0; i < Key::nlen; ++i)
	{
		memcpy(new_data + size, val+i, sizeof(val[i]));
		size += (uint32_t)sizeof(val[i]);
	}

	data = new_data;

	return *this;
}

/* T_ADDR */
pf_addr Packet::ReadAddr()
{
	ASSERT(size >= pf_addr::size);

	pf_addr val(data);

	char* new_data;
	size -= (uint32_t)pf_addr::size;
	if(size > 0)
	{
		new_data = new char [size];
		memcpy(new_data, data + pf_addr::size, size);
	}

	delete []data;

	if(size > 0)
		data = new_data;
	else
		data = NULL;

	return val;
}

Packet& Packet::Write(pf_addr addr)
{
	ASSERT(((uint32_t)pf_addr::size) + size >= size);
	char* new_data = new char [size + pf_addr::size];

	if(data)
		memcpy(new_data, data, size);
	if(data)
		delete []data;

	addr.dump(new_data + size);
	size += (uint32_t)pf_addr::size;
	data = new_data;

	return *this;
}

/* T_ADDRLIST */
AddrList Packet::ReadAddrList()
{
	AddrList addr_list;
	uint32_t list_size = ReadInt32();

	ASSERT((size * pf_addr::size) >= list_size);

	for(uint32_t i = 0; i < list_size; ++i)
		addr_list.push_back(ReadAddr());

	return addr_list;
}

Packet& Packet::Write(const AddrList& addr_list)
{
	ASSERT(addr_list.size() <= UINT_MAX);
	Write((uint32_t)addr_list.size());

	ASSERT((addr_list.size() * pf_addr::size) + size >= size);

	for(AddrList::const_iterator it = addr_list.begin(); it != addr_list.end(); ++it)
		Write(*it);

	return *this;
}

/* T_STR */
std::string Packet::ReadStr()
{
	uint32_t str_size = ReadInt32();
	ASSERT(size >= str_size);
	char* str = new char [str_size+1];

	memcpy(str, data, str_size);
	str[str_size] = '\0';
	std::string val = std::string(str);

	char* new_data;
	size -= str_size;
	if(size > 0)
	{
		new_data = new char [size];
		memcpy(new_data, data + str_size, size);
	}

	delete []data;
	delete []str;

	if(size > 0)
		data = new_data;
	else
		data = NULL;

	return val;
}

Packet& Packet::Write(const std::string& str)
{
	ASSERT(str.size() <= UINT_MAX);

	uint32_t str_len = (uint32_t)str.size();
	ASSERT(str_len + size >= size);

	Write(str_len);

	char* new_data = new char [size + str_len];
	if(data)
		memcpy(new_data, data, size);
	if(data)
		delete []data;

	memcpy(new_data + size, str.c_str(), str_len);
	size += str_len;
	data = new_data;

	return *this;
}

/* T_CHUNK */
FileChunk Packet::ReadChunk()
{
	off_t c_offset = ReadInt64();
	uint32_t c_size = ReadInt32();

	FileChunk chunk(data, c_offset, c_size);

	char* new_data;
	size -= c_size;
	if(size > 0)
	{
		new_data = new char [size];
		memcpy(new_data, data + c_size, size);
	}

	delete []data;

	if(size > 0)
		data = new_data;
	else
		data = NULL;

	return chunk;
}

Packet& Packet::Write(FileChunk chunk)
{
	ASSERT(chunk.GetSize() <= UINT_MAX);
	ASSERT(((uint32_t)sizeof(chunk.GetOffset()) + ((uint32_t)sizeof(chunk.GetSize())) + chunk.GetSize()) + size >= size);
	Write((uint64_t)chunk.GetOffset());
	Write((uint32_t)chunk.GetSize());

	char* new_data = new char [size + chunk.GetSize()];

	if(data)
		memcpy(new_data, data, size);
	if(data)
		delete []data;

	char* ptr = new_data + size;
	memcpy(ptr, chunk.GetData(), chunk.GetSize());

	size += (uint32_t)chunk.GetSize();
	data = new_data;

	return *this;
}
