/*
 * Copyright(C) 2012 Michael Muré <batolettre@gmail.com>
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

#include <dht/dht.h>
#include <dht/messages.h>
#include <chimera/chimera.h>
#include <net/hosts_list.h>
#include <scheduler/scheduler.h>
#include <util/pf_log.h>

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " listen_port [boostrap_host:port]" << std::endl;
		return EXIT_FAILURE;
	}

	srand(time(NULL));

	DHT* dht = new DHT(StrToTyp<uint16_t>(argv[1]));

	pf_log.SetLoggedFlags("ALL", false);
	Scheduler::StartSchedulers(5);

	if(argc > 2)
	{
		Host host = hosts_list.DecodeHost(argv[2]);
		pf_log[W_INFO] << "Connecting to " << host;
		dht->GetChimera()->Join(host);
	}

	std::string s;
	while(std::getline(std::cin, s))
	{
		std::string command_str = stringtok(s, " ");
		Key k;

		if(command_str.length() == 0)
			continue;

		switch(command_str[0])
		{
			case 'p':
			case 'P':
				k.MakeHash(s);
				pf_log[W_INFO] << "Publish " << s << " with key " << k;
				dht->Publish(k, s);
				break;
			case 'u':
			case 'U':
				k = stringtok(s, " ");
				pf_log[W_INFO] << "Unublish " << s << " with key " << k;
				dht->Unpublish(k, s);
				break;
			case 'g':
			case 'G':
				k = s;
				pf_log[W_INFO] << "Request data with key " << k;
				dht->RequestData(k);
				break;
			case 'q':
			case 'Q':
				return EXIT_SUCCESS;
			default:
				pf_log[W_ERR] << "Command not recognized.";
		}
	}

	return EXIT_SUCCESS;
}
