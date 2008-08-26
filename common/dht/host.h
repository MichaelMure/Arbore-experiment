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

#ifndef _HOST_H_
#define _HOST_H_

#include <netinet/in.h>

#include "key.h"
#include "pf_log.h"

#define SUCCESS_WINDOW 20
#define GOOD_LINK 0.8
#define BAD_LINK 0.3

class _Host;
class Mutex;

class Host
{
	_Host* host;

public:

	Host(Mutex* mutex, const pf_addr& addr);
	Host(const Host&);
	Host& operator=(const Host&);
	~Host();

	in_addr_t GetAddress() const;

	/** host_encode:
	 ** encodes the #host# into a string, putting it in #s#, which has
	 ** #len# bytes in it.
	 */
	std::string Encode() const;

	/** host_update_stat:
	 ** updates the success rate to the host based on the SUCCESS_WINDOW average
	 */
	void UpdateStat (int success);

	const Key& GetKey() const;
	void SetKey(Key k);

	const std::string& GetName() const;
	int GetPort() const;
	double GetFailureTime() const;
	double GetLatency() const;

	void SetFailureTime(double f);
	float GetSuccessAvg() const;
};


template<>
inline Log::flux& Log::flux::operator<< <Host> (Host host)
{
	str += host.GetName() + ":" + TypToStr(host.GetPort());
	return *this;
}


#endif /* _HOST_H_ */
