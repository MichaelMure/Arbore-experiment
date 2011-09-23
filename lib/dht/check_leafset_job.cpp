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

#include <net/packet.h>
#include <util/pf_log.h>
#include "check_leafset_job.h"
#include "chimera.h"
#include "chimera_messages.h"
#include "chimera_routing.h"

bool CheckLeafsetJob::Start()
{
	std::string s;

	std::vector<Host> leafset = routing->getLeafset();
	for (std::vector<Host>::iterator it = leafset.begin(); it != leafset.end(); ++it)
	{
		if (*it && !chimera->Ping(*it))
		{
			it->SetFailureTime(dtime ());
			pf_log[W_WARNING] << "message send to host: " << *it
					  << " failed at time: " << it->GetFailureTime() << "!";
			if (it->GetSuccessAvg() < BAD_LINK)
			{
				pf_log[W_DEBUG] << "Deleting " << *it;
				routing->remove(*it);
			}
		}
	}
	std::vector<Host> table = routing->getRoutingTable();
	for (std::vector<Host>::iterator it = table.begin(); it != table.end(); ++it)
	{
		if (*it && !chimera->Ping(*it))
		{
			it->SetFailureTime(dtime ());
			pf_log[W_WARNING] << "message send to host: " << *it
					  << " failed at time: " << it->GetFailureTime() << "!";
			if (it->GetSuccessAvg() < BAD_LINK)
			{
				pf_log[W_DEBUG] << "Deleting " << *it;
				routing->remove(*it);
			}
		}
	}

	/* send leafset exchange data every  3 times that pings the leafset */
	if (count == 2)
	{
		Packet pckt(ChimeraPiggyType, chimera->GetMe().GetKey());
		std::vector<Host> leafset = routing->getLeafset();

		leafset.push_back(chimera->GetMe());
		count = 0;

		AddrList addrlist;
		for(std::vector<Host>::iterator it = leafset.begin(); it != leafset.end(); ++it)
			addrlist.push_back(it->GetAddr());

		pckt.SetArg(CHIMERA_PIGGY_ADDRESSES, addrlist);

		for (std::vector<Host>::iterator it = leafset.begin(); it != leafset.end(); ++it)
		{
			pckt.SetDst(it->GetKey());
			if(!chimera->Send(*it, pckt))
			{
				pf_log[W_WARNING] << "sending leafset update to " << *it << " failed!";
				if (it->GetSuccessAvg() < BAD_LINK)
				{
					routing->remove(*it);
				}
			}
		}
	}
	else
	{
		count++;
	}

	return true;
}
