
#define FUSE_USE_VERSION 26

#include <iostream>
#include <cstdlib>
#include <fuse.h>
#include <sys/resource.h>

#include "libconfig.h"
#include "application.h"
#include "network.h"
#include "cache.h"
#include "log.h"
#include "pf_file.h"
#include "pf_fuse.h"
#include "session_config.h"

MyConfig conf;

Application::Application()
{
	ConfigSection* section = conf.AddSection("listen", "Listening server", false);
	section->AddItem(new ConfigItem_string("bind", "IP Binding"));
	section->AddItem(new ConfigItem_int("port", "Port", 1, 65535));

	section = conf.AddSection("connection", "Connect to this server at launch", true);
	section->AddItem(new ConfigItem_string("host", "Hostname"), true);
	section->AddItem(new ConfigItem_int("port", "Port", 1, 65535));

	section = conf.AddSection("logging", "Log informations", false);
	section->AddItem(new ConfigItem_string("level", "Logging level"));

	section = conf.AddSection("hdd", "Harddisk configuration", false);
	section->AddItem(new ConfigItem_string("root", "Root directory of storage"));
	section->AddItem(new ConfigItem_string("workdir", "Root directory of config files"));
}

int Application::main(int argc, char *argv[])
{
	if(argc < 2)
	{
		std::cerr << "Syntax: " << argv[0] << " <configpath> <mount point> [fuse options]" << std::endl;
		exit(EXIT_FAILURE);
	}

	struct rlimit rlim;
	if(!getrlimit(RLIMIT_CORE, &rlim) && rlim.rlim_cur != RLIM_INFINITY)
	{
	        rlim.rlim_cur = RLIM_INFINITY;
	        rlim.rlim_max = RLIM_INFINITY;
	        setrlimit(RLIMIT_CORE, &rlim);
	}


	try
	{
		if(!conf.Load(argv[1]))
		{
			log[W_ERR] << "Unable to load configuration, exiting..";
			exit(EXIT_FAILURE);
		}
		log.SetLoggedFlags(conf.GetSection("logging")->GetItem("level")->String());

		session_cfg.Load(conf.GetSection("hdd")->GetItem("workdir")->String() + "/session.cfg");

		cache.Load(conf.GetSection("hdd")->GetItem("root")->String());
		cache.UpdateRespFiles();

		net.Start(&conf);

		umask(0);
		return fuse_main(argc-1, argv+1, &pf_oper, NULL);
	}
	catch(MyConfig::error &e)
	{
		log[W_ERR] << "Error while loading:";
		log[W_ERR] << e.Reason();
	}
	catch(Network::CantRunThread &e)
	{
		log[W_ERR] << "Unable to create network thread, exiting..";
	}
	catch(Network::CantOpenSock &e)
	{
		log[W_ERR] << "Unable to open socket, exiting..";
	}
	catch(Network::CantListen &e)
	{
		log[W_ERR] << "Unable to liston on port " << e.port << ", exiting..";
	}
	catch(Network::CantResolvHostname &e)
	{
		/* NOTE: This must never arrive because when I can't connect to server, I try
		 * an other server, or I wait alone..
		 */
		log[W_ERR] << "Unable to resolve hostname, exiting..";
	}
	catch(Hdd::HddAccessFailure &e)
	{
		log[W_ERR] << "Can't access cache root directory: " << e.dir;
	}
	catch(Hdd::HddWriteFailure &e)
	{
		log[W_ERR] << "Can't write to cache: " << e.dir;
	}

	exit(EXIT_FAILURE);

}

