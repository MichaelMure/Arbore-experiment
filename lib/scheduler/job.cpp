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
#include "job.h"

Job::Job(double start_at, repeat_type_t _repeat_type, double _repeat_delta)
 : start_time(start_at),
   repeat_type(_repeat_type),
   repeat_delta(_repeat_delta)
{
}

bool Job::DoStart()
{
	bool ret = Start();

	if(ret)
	{
		// Update the start-time
		switch(repeat_type)
		{
			case REPEAT_NONE:
				return false;
			case REPEAT_PERIODIC:
				start_time = time::dtime() + repeat_delta;
				break;
			case REPEAT_LESS_AND_LESS:
				start_time = time::dtime() + repeat_delta;
				repeat_delta *= 2;
				break;
			default:
				return false;
		}
	}
	return ret;
}

double Job::GetStartTime() const
{
  return start_time;
}
