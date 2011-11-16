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

#include <stdlib.h>
#include <string>
#include <iostream>

#include <net/network.h>
#include <net/packet.h>
#include <net/hosts_list.h>
#include <dht/chimera.h>
#include <scheduler/scheduler.h>
#include <util/pf_log.h>
#include <util/tools.h>
#include <dht/chimera_messages.h>

enum
{
	CHIMERA_CHAT_MESSAGE
};

class ChimeraChatMessage : public ChimeraBaseMessage
{
public:
	void Handle(ChimeraDHT&, const Host&, const Packet& pckt)
	{
		std::string message = pckt.GetArg<std::string>(CHIMERA_CHAT_MESSAGE);
		pf_log[W_INFO] << "CHAT[" << pckt.GetSrc() << "] " << message;
	}
};

int main(int argc, char** argv)
{
	PacketType ChimeraChatType(15, new ChimeraChatMessage, Packet::REQUESTACK|Packet::MUSTROUTE, "CHAT", T_STR, T_END);

	if(argc < 2 || argc > 3)
	{
		std::cout << "Usage: " << argv[0] << " listen_port [boostrap_host] [port]" << std::endl;
		return EXIT_FAILURE;
	}

	Network net;
	Key me(StrToTyp<uint32_t>(argv[1]));

	ChimeraDHT* dht = new ChimeraDHT(&net, StrToTyp<uint16_t>(argv[1]), me);
	dht->RegisterType(ChimeraChatType);

	pf_log[W_INFO] << "hosts_list pointer: " << dht->GetNetwork()->GetHostsList();

	pf_log.SetLoggedFlags("ALL", false);
	Scheduler::StartSchedulers(5);
	net.Start();

	Host host;

	if(argc == 2)
	{
		host = net.GetHostsList()->GetHost((std::string) argv[2]);
	}
	else if(argc==3)
	{
		host = net.GetHostsList()->GetHost((std::string) argv[2], StrToTyp<uint16_t>(argv[3]));
	}

	pf_log[W_INFO] << "Connecting to " << host;
	dht->Join(host);

	std::string s;
	while(std::getline(std::cin, s))
	{
		Key key = Key(stringtok(s, " "));

		Packet pckt(ChimeraChatType, dht->GetMe().GetKey(), key);
		pckt.SetFlag(Packet::MUSTROUTE);
		pckt.SetArg(CHIMERA_CHAT_MESSAGE, s);
		pf_log[W_INFO] << "CHAT[" << key << "] " << s;
		dht->Route(pckt);
	}

	return 0;
}
