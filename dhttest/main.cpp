#include <stdlib.h>
#include <string>
#include <iostream>

#include "common/net/network.h"
#include "common/net/packet.h"
#include "common/net/hosts_list.h"
#include "common/dht/chimera.h"
#include "common/scheduler.h"
#include "common/pf_log.h"
#include "common/tools.h"
#include "common/dht/chimera_messages.h"

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
	PacketType ChimeraChatType(++LastChimeraType, new ChimeraChatMessage,  "CHAT", T_STR, T_END);

	if(argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " port [host]" << std::endl;
		return EXIT_FAILURE;
	}

	Network net;
	Key me(StrToTyp<uint32_t>(argv[1]));

	ChimeraDHT* dht = new ChimeraDHT(&net, StrToTyp<int>(argv[1]), me);
	dht->RegisterType(ChimeraChatType);

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
