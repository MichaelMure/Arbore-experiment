#include <stdlib.h>
#include <iostream>

#include "common/net/network.h"
#include "common/net/hosts_list.h"
#include "common/dht/chimera.h"
#include "common/scheduler.h"
#include "common/pf_log.h"
#include "common/tools.h"

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " port [host]" << std::endl;
		return EXIT_FAILURE;
	}

	Network net;
	ChimeraDHT* dht;
	Key me(StrToTyp<uint32_t>(argv[1]));

	dht = new ChimeraDHT(&net, StrToTyp<int>(argv[1]), me);

	pf_log.SetLoggedFlags("ALL", false);
	Scheduler::StartSchedulers(5);
	net.Start();

	if(argc > 2)
	{
		Host host = net.GetHostsList()->DecodeHost(argv[2]);
		pf_log[W_INFO] << "Connecting to " << host;
		dht->Join(host);
	}

	while(1) sleep(1);


	return 0;
}
