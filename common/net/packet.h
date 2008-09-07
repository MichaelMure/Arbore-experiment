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

#ifndef PACKET_H
#define PACKET_H

#include <vector>
#include <cassert>
#include <string>
#include "pf_types.h"
#include "packet_arg.h"
#include "packet_type.h"
#include "packet_type_list.h"
#include "files/file_chunk.h"
#include "pf_addr.h"

typedef std::vector<pf_addr> AddrList;

class PacketTypeList;

/** \brief the Packet's class.
 *
 * This class can be used to represent a packet which
 * can be sent or received to/from an other peer.
 *
 * The physical structure of a packet is:
 *
 * .----------.----------.----------.
 * |    src   |    dst   |   type   |
 * |  uint32  |  uint32  |  uint32  |
 * |----------+----------+----------|
 * |   size   |   nseq   |  flags   |
 * |  uint32  |  uint32  |  uint32  |
 * |----------'----------'----------|
 * |                                |
 * |             data ...           |
 * |                                |
 * '--------------------------------'
 *
 * All arguments are not separated. To read a
 * packet content you have to know what
 * are arguments of this packet type.
 *
 * This is why you have to give a PacketType object,
 * or a pointer to a PacketTypeList object in
 * the constructor which gets data buffer.
 */
class Packet
{
	std::vector<PacketArgBase*> arg_lst;

	void BuildArgsFromData();
	void BuildDataFromArgs();

	// Writing to buffer functions
	Packet& Write(uint32_t nbr);
	Packet& Write(uint64_t nbr);
	Packet& Write(Key nbr);
	Packet& Write(pf_addr addr);
	Packet& Write(const std::string& str);
	Packet& Write(const AddrList& addr_list);
	Packet& Write(FileChunk chunk);

	// Reading from buffer functions
	uint32_t ReadInt32();
	uint64_t ReadInt64();
	Key ReadKey();
	pf_addr ReadAddr();
	std::string ReadStr();
	AddrList ReadAddrList();
	FileChunk ReadChunk();

protected:
	PacketType type;
	uint32_t size;			  // size of the msg (excluding header)
	Key src;
	Key dst;
	char* data;

public:

	/** Exception raised when the packet is malformated */
	class Malformated : public std::exception {};

	/** Constructor to build a new packet.
	 *
	 * @param type  this is the PacketType object which described the type of packet.
	 * @param src  sender's key of the packet.
	 * @param dst  receiver's key of the packet.
	 */
	Packet(PacketType type, const Key& src = Key(), const Key& dst = Key());

	/** Copy constructor.
	 *
	 * @param packet  the Packet object which is copied.
	 */
	Packet(const Packet& packet);

	/** Constructor to build the Packet object from data.
	 *
	 * It gets the packet's header, find the type and wait for
	 * the \b SetContent method call to fill args.
	 *
	 * @param pckt_type_list  This is the PacketTypeList object pointer which
	 *                        is used to get the PacketType object from type
	 *                        contained in header.
	 * @param header  the header data (size must be GetHeaderSize()).
	 */
	Packet(PacketTypeList* pckt_type_list, char* header);

	/** Copy operator.
	 *
	 * @param packet  the Packet object which is copied.
	 */
	Packet& operator=(const Packet& packet);

	/** Packet destructor */
	~Packet();

	/** Get the data of the packet.
	 *
	 * You *must* free memory.
	 */
	char* DumpBuffer();

	/** Set content of packet.
	 *
	 * It *must* be used after use of the constructor which
	 * ask a header buffer.
	 *
	 * @param buf  content buffer of packet.
	 * @param _size  size of buffer. It *must* be GetDataSize().
	 */
	void SetContent(const char* buf, size_t _size);

	/** Returns the header's size.
	 *
	 * The header's size is constant, so this is a static method.
	 */
	static uint32_t GetHeaderSize();

	/** Get total size of packet.
	 *
	 * @return  GetDataSize() + GetHeaderSize().
	 */
	uint32_t GetSize() const;

	/** Get the data size. */
	uint32_t GetDataSize() const { return size; }

	/** Get the integer type of packet */
	uint32_t GetType() const { return type.GetType(); }

	/** Returns the sender's key. */
	Key GetSrc() const { return src; }

	/** Returns the destination's key. */
	Key GetDst() const { return dst; }

	/** Set the sender's key. */
	Packet& SetSrc(Key id) { src = id; return *this; }

	/** Set the destination's key. */
	Packet& SetDst(Key id) { dst = id; return *this; }

	/** Get a string which represents the packet info.
	 *
	 * @return  a string in form:
	 *          [src->dst] <type's string> arg1, arg2, arg3, ...
	 */
	virtual std::string GetPacketInfo() const;

	/** Set the argument's value.
	 *
	 * This template function msut be used to set the argument's value
	 * at any position.
	 *
	 * @param arg  the argument's position.
	 * @param val  value
	 */
	template<typename T>
	void SetArg(size_t arg, T val)
	{
		if(arg_lst.size() <= arg)
			arg_lst.resize(arg + 1, NULL);
		if(arg_lst[arg] != NULL)
			delete arg_lst[arg];

		arg_lst[arg] = new PacketArg<T>(val);
	}

	/** Get an argument value.
	 *
	 * This template method can be used to get the value
	 * of any argument.
	 *
	 * @param arg  the argument's position.
	 * @return  the value
	 */
	template<typename T>
	T GetArg(size_t arg) const
	{
		assert(arg_lst.size() > arg);
		assert(arg_lst[arg] != NULL);
		assert(dynamic_cast< PacketArg<T>* >(arg_lst[arg]));

		return (dynamic_cast< PacketArg<T>* >(arg_lst[arg]))->val;
	}
};
#endif						  /* PACKET_H */
