#include <stdlib.h>
#include <string>
#include <iostream>

#include "lib/net/network.h"
#include "lib/net/packet.h"
#include "lib/net/hosts_list.h"
#include "lib/dht/chimera.h"
#include "lib/scheduler/scheduler.h"
#include "lib/util/pf_log.h"
#include "lib/util/tools.h"
#include "lib/dht/chimera_messages.h"

enum
{
	CHIMERA_CHAT_MESSAGE
};

class ChimeraChatMessage : public ChimeraBaseMessage
{
public:
	void Handle(ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		std::string message = pckt.GetArg<std::string>(CHIMERA_CHAT_MESSAGE);
		pf_log[W_INFO] << "CHAT[" << pckt.GetSrc() << "] " << message;
	}
};

int main(int argc, char** argv)
{
	PacketType ChimeraChatType(15, new ChimeraChatMessage, Packet::REQUESTACK|Packet::MUSTROUTE, "CHAT", T_STR, T_END);

	if(argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " port [host]" << std::endl;
		return EXIT_FAILURE;
	}

	Network net;
	Key me(StrToTyp<uint32_t>(argv[1]));

	ChimeraDHT* dht = new ChimeraDHT(&net, StrToTyp<int>(argv[1]), me);
	dht->RegisterType(ChimeraChatType);

	std::cerr << "hosts_list pointer: " << dht->GetNetwork()->GetHostsList() << std::endl;

	pf_log.SetLoggedFlags("ALL", false);
	Scheduler::StartSchedulers(5);
	net.Start();

	if(argc > 2)
	{
		Host host = net.GetHostsList()->DecodeHost(argv[2]);
		pf_log[W_INFO] << "Connecting to " << host;
		dht->Join(host);
	}

	std::string s;
	while(std::getline(std::cin, s))
	{
		std::string keystr = stringtok(s, " ");
		Key key;
		key = keystr;

		Packet pckt(ChimeraChatType, dht->GetMe().GetKey(), key);
		pckt.SetFlag(Packet::MUSTROUTE);
		pckt.SetArg(CHIMERA_CHAT_MESSAGE, s);
		pf_log[W_INFO] << "CHAT[" << key << "] " << s;
		dht->Route(pckt);
	}

	return 0;
}
