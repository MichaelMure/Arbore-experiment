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

#include <util/pf_log.h>
#include <util/key.h>
#include "pf_addr.h"

#define SUCCESS_WINDOW 20
#define GOOD_LINK 0.8
#define BAD_LINK 0.3

class _Host;
class Mutex;

class Host
{
	_Host* host;  /**< This is the private _Host object contained in all Host copied from it. */

	void deinit();

public:

	/** Empty constructor */
	Host();

	/** Constructor of Host.
	 *
	 * @param mutex  to be thread-safe, we have to lock a mutex while doing something.
	 * @param addr  this is the address representation of this host.
	 */
	Host(Mutex* mutex, const pf_addr& addr);

	/** \brief The copy constructor.
	 *
	 * @param host  this is Host object which is copied from.
	 *
	 * \note when you copy two Host objects, the same _Host contained object
	 * will be referenced.
	 */
	Host(const Host& host);

	/** \brief The copy constructor.
	 *
	 * @param other  this is Host object which is copied from.
	 *
	 * \note when you copy two Host objects, the same _Host contained object
	 * will be referenced.
	 */
	Host& operator=(const Host&);

	/** \brief Destructor.
	 *
	 * If the _Host contained object is only referenced by me, I delete it.
	 * In other case, we do nothing.
	 */
	~Host();

	/** \brief Comparaison operator
	 *
	 * @param host  host compared to.
	 *
	 * Check if two Host objects contains same _Host object.
	 */
	bool operator==(const Host& host);

	/** \brief The ! operator
	 *
	 * @return  true if the host isn't valid (without any _Host object).
	 */
	bool operator!() const;

	/** \brief The bool convertion.
	 *
	 * @return  true if the host is valid (contains a _Host object).
	 */
	operator bool() const;

	/** \brief Get the address.
	 *
	 * @return a pf_addr object.
	 */
	pf_addr GetAddr() const;

	/** \brief Encodes me into a string.
	 *
	 * TODO: probably useless
	 */
	std::string Encode() const;

	 /** \brief updates the success rate to the host based on the SUCCESS_WINDOW average */
	void UpdateStat (int success);

	Key GetKey() const;                   /**< Get the key */
	void SetKey(Key k);                   /**< Set the key */

	double GetFailureTime() const;        /**< Get the failure time */
	void SetFailureTime(double f);        /**< Set the failure time */

	double GetLatency() const;            /**< Get the latency */
	void SetLatency(double l);            /**< Set the latency */

	float GetSuccessAvg() const;          /**< Get the success average */

	/** \brief Get the reference count.
	 *
	 * @return  an unsigned int which represents the reference count
	 *          for the _Host object.
	 */
	unsigned int GetReference() const;
};


template<>
inline Log::flux& Log::flux::operator<< <Host> (Host host)
{
	_str += host.GetAddr().GetStr();
	return *this;
}

extern const Host InvalidHost;

#endif /* _HOST_H_ */
