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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "net/pf_addr.h"
#include "host.h"

class _Host
{
	Mutex* mutex;
	pf_addr addr;

	int failed;
	double failuretime;
	double latency;
	double loss;
	double success;
	int success_win[SUCCESS_WINDOW];
	int success_win_index;
	float success_avg;
	Key key;

public:
	unsigned int reference;

	_Host(Mutex* mutex, const pf_addr& addr);

	pf_addr GetAddr() const { return addr; }

	/** host_encode:
	 ** encodes the #host# into a string, putting it in #s#, which has
	 ** #len# bytes in it.
	 */
	std::string Encode() const;

	Mutex* GetMutex() const { return mutex; }

	/** host_update_stat:
	 ** updates the success rate to the host based on the SUCCESS_WINDOW average
	 */
	void UpdateStat (int success);

	const Key& GetKey() const { return key; }
	void SetKey(Key k) { key = k; }

	double GetFailureTime() const { return failuretime; }
	double GetLatency() const { return latency; }

	void SetFailureTime(double f) { failuretime = f; }
	float GetSuccessAvg() const { return success_avg; }

};

_Host::_Host(Mutex* _mutex, const pf_addr& _addr)
	: mutex(_mutex),
	addr(_addr),
	failed(0),
	failuretime(0),
	latency(0),
	loss(0),
	success(0),
	success_win_index(0),
	success_avg(0.5),
	key(0)
{
	size_t i;
	for (i = 0; i < SUCCESS_WINDOW / 2; i++)
		success_win[i] = 0;
	for (i = SUCCESS_WINDOW / 2; i < SUCCESS_WINDOW; i++)
		success_win[i] = 1;
}

/** host_encode:
 ** encodes the #host# into a string, putting it in #s#, which has
 ** #len# bytes in it.
 */
std::string _Host::Encode() const
{
	return addr.str();
}

/** host_update_stat:
 ** updates the success rate to the host based on the SUCCESS_WINDOW average
 */
void _Host::UpdateStat (int success)
{

	int i;
	float total = 0;

	this->success_win[this->success_win_index++ % SUCCESS_WINDOW] = success;
	this->success_avg = 0;

	// printf("SUCCESS_WIN[");
	for (i = 0; i < SUCCESS_WINDOW; i++)
	{
		//  printf("%i ",this->success_win[i]);
		total += (float)this->success_win[i];
		//   this->success_avg = this->success_win[i]/SUCCESS_WINDOW;
	}
	// printf("]   ");
	this->success_avg = total / SUCCESS_WINDOW;
	//  printf("Total: %f, avg: %f\n",total,this->success_avg);

}

Host::Host(Mutex* mutex, const pf_addr& addr)
{
	BlockLockMutex(this->host->GetMutex());
	this->host = new _Host(mutex, addr);
}

Host::Host(const Host& h)
{
	BlockLockMutex(this->host->GetMutex());
	this->host = h.host;
	this->host->reference++;
}

Host& Host::operator=(const Host& h)
{
	BlockLockMutex(this->host->GetMutex());
	this->host = h.host;
	this->host->reference++;

	return *this;
}

Host::~Host()
{
	BlockLockMutex(this->host->GetMutex());
	this->host->reference--;
	if(!this->host->reference)
		delete this->host;
}

pf_addr Host::GetAddr() const
{
	BlockLockMutex(this->host->GetMutex());
	return host->GetAddr();
}

/** host_encode:
 ** encodes the #host# into a string, putting it in #s#, which has
 ** #len# bytes in it.
 */
std::string Host::Encode() const
{
	BlockLockMutex(this->host->GetMutex());
	return host->Encode();
}

/** host_update_stat:
 ** updates the success rate to the host based on the SUCCESS_WINDOW average
 */
void Host::UpdateStat (int success)
{
	BlockLockMutex(this->host->GetMutex());
	host->UpdateStat(success);
}

const Key& Host::GetKey() const
{
	BlockLockMutex(this->host->GetMutex());
	return host->GetKey();
}
void Host::SetKey(Key k)
{
	BlockLockMutex(this->host->GetMutex());
	host->SetKey(k);
}

double Host::GetFailureTime() const
{
	BlockLockMutex(this->host->GetMutex());
	return host->GetFailureTime();
}
double Host::GetLatency() const
{
	BlockLockMutex(this->host->GetMutex());
	return host->GetLatency();
}

void Host::SetFailureTime(double f)
{
	BlockLockMutex(this->host->GetMutex());
	host->SetFailureTime(f);
}
float Host::GetSuccessAvg() const
{
	BlockLockMutex(this->host->GetMutex());
	return host->GetSuccessAvg();
}

unsigned int Host::GetReference() const
{
	BlockLockMutex(this->host->GetMutex());
	return host->reference;
}
