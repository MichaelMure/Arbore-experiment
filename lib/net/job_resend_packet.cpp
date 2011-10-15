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

#include <util/time.h>
#include "job_resend_packet.h"

bool ResendPacketJob::Start()
{
	if(!network->Send(sock, desthost, packet))
		return false;
	retry++;
	if(retry < Network::MAX_RETRY)
		return true;

	desthost.UpdateStat(0);
	return false;
}

ResendPacketJob::ResendPacketJob(Network* _network, int _sock, const Host& _desthost, const Packet& _packet, double transmit_time)
	: Job(time::dtime(), REPEAT_NONE, Network::RETRANSMIT_INTERVAL),
		sock(_sock),
		desthost(_desthost),
		packet(_packet),
		retry(0),
		transmittime(transmit_time),
		network(_network)
{}

const Packet& ResendPacketJob::GetPacket() const
{
	return packet;
}

double ResendPacketJob::GetTransmitTime() const
{
	return transmittime;
}

Host ResendPacketJob::GetDestHost() const
{
	return desthost;
}
