/*
 * Copyright(C) 2012 Michael Muré
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

#ifndef CLEAN_STORAGE_JOB_H
#define CLEAN_STORAGE_JOB_H

#include <scheduler/job.h>
#include <util/time.h>

class Storage;

/** Class to clean periodicly the DHT storage, to remove old values */
class CleanStorageJob : public Job
{
public:

	CleanStorageJob(Storage* storage)
		: Job(time::dtime(), REPEAT_PERIODIC, 20.0),
		  storage_(storage)
	{}

private:
	bool Start();

	Storage* storage_;
};

#endif /* CLEAN_STORAGE_JOB_H */
