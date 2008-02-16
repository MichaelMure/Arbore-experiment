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

#define FUSE_USE_VERSION 26

#include <iostream>
#include <cstdlib>
#include <fuse.h>
#include <sys/resource.h>

#include "libconfig.h"
#include "network.h"
#include "cache.h"
#include "log.h"
#include "pf_file.h"
#include "pf_fuse.h"
#include "session_config.h"

MyConfig conf;

class Application
{
public:
	Application()
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

	int main(int argc, char *argv[])
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
			cache.Load(conf.GetSection("hdd")->GetItem("root")->String());

			session_cfg.Load(conf.GetSection("hdd")->GetItem("workdir")->String() + "/session.cfg");

			uint32_t my_id;
			if(session_cfg.Get("my_id", my_id))
				net.SetMyID((id_t)my_id);

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
		catch(Peer::PeerCantConnect &e)
		{
			log[W_ERR]  << "Host " << e.addr << " can't connect.";
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

};

int main(int argc, char** argv)
{
	Application app;
	return app.main(argc, argv);
}

