/*
 * Copyright(C) 2012 Benoît Saccomano
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

#include <files/arbore.h>
#include <dht/messages.h>
#include <files/file_chunk.h>
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

	Key me(StrToTyp<uint32_t>(argv[1]));

	Arbore* arbore = new Arbore(StrToTyp<uint16_t>(argv[1]));

	pf_log.SetLoggedFlags("DESYNCH WARNING ERR INFO DHT FILE", false);
	Scheduler::StartSchedulers(5);

	if(argc > 2)
	{
		Host host = hosts_list.DecodeHost(argv[2]);
		pf_log[W_INFO] << "Connecting to " << host;
		arbore->GetDHT()->GetChimera()->Join(host);
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
			case 's':
			case 'S':
			{
				pf_log[W_FILE] << "Try to create a chunk";
				char chaine[10]={ 'B', 'o', 'n', 'j', 'o', 'u', 'r', '\0' };
				char *data = (char*) &chaine;
				FileChunk* fc = new FileChunk(data, 0, sizeof(chaine));
				k = s;
				pf_log[W_FILE] << "Try to send the chunk" << " with key " << k;
				arbore->Send(k, *fc);
				break;
			}
			case 'q':
			case 'Q':
				return EXIT_SUCCESS;
			default:
				pf_log[W_ERR] << "Command not recognized.";
		}
	}


	return EXIT_SUCCESS;
}
