/*
 * Copyright(C) 2008 Romain Bignon
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
 * This file contains some code from the Chimera's Distributed Hash Table,
 * written by CURRENT Lab, UCSB.
 *
 */

#include "check_leafset_job.h"
#include "chimera_routing.h"
#include "pf_log.h"
#include "chimera.h"

bool CheckLeafsetJob::Start()
{
	std::string s;
	size_t i, count = 0;

	std::vector<Host> leafset = chimera->routing->getLeafset();
	//TODO (eld) use an iterator instead ?
	for (i = 0; i < leafset.size(); i++)
	{
		if (state->Ping(leafset[i]) == 1)
		{
			leafset[i].SetFailureTime(dtime ());
			pf_log[W_WARNING] << "message send to host: " << leafset[i].GetName() << ":" << leafset[i].GetPort()
					  << " failed at time: " << leafset[i].GetFailureTime() << "!";
			if (leafset[i].GetSuccessAvg() < BAD_LINK)
			{
				printf ("Deleting %s:%d \n",
				    leafset[i].GetName(),
				    leafset[i].GetPort());
				chimera->routing->remove(leafset[i]);
			}
		}
		state->host->Release(leafset[i]);
	}
	std::vector<Host> table = chimera->routing->getRoutingTable();
	for (i = 0; i < table.size(); i++)
	{
		if (state->Ping(table[i]) == 1)
		{
			table[i]->SetFailureTime(dtime ());
			pf_log[W_WARNING] << "message send to host: " << table[i].GetName() << ":" << table[i].GetPort()
					  << " failed at time: " << table[i].GetFailureTime() << "!";
			if (table[i].GetSuccessAvg() < BAD_LINK)
			{
				chimera->routing->remove(table[i]);
				//route_update (state, table[i], 0);
			}
		}
		state->host->Release(table[i]);
	}

	/* send leafset exchange data every  3 times that pings the leafset */
	if (count == 2)
	{
		count = 0;
		std::vector<Host> leafset = chimera->routing->getLeafset();
		chglob->me->Encode(s, NETWORK_PACK_SIZE);
		strcat (s, "\n");	/* add a spacer */
		state->encode_hosts (s + strlen (s), NETWORK_PACK_SIZE - strlen (s),
				     leafset);
		//TODO (eld) use iterator instead ?
		for (i = 0; i < leafset.size(); i++)
		{
			m = new Message (leafset[i].GetKey(),
					CHIMERA_PIGGY, strlen (s) + 1,
					s);
			if (!state->message->message_send (leafset[i], m, true))
			{
				pf_log[W_WARNING] << "sending leafset update to "
						  << leafset[i].GetName() << ":"
						  << leafset[i].GetPort() << " failed!";
				if (leafset[i].GetSuccessAvg() < BAD_LINK)
				{
					chimera->routing->remove(leafset[i]);
				}
			}
			delete m;
			state->host->Release(leafset[i]);
		}
	}
	else
	{
		count++;
	}

	return true;
}
