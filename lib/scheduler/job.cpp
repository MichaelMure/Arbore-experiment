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

#include <util/dtime.h>
#include "job.h"

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
				start_time = dtime() + repeat_delta;
				break;
			case REPEAT_LESS_AND_LESS:
				start_time = dtime() + repeat_delta;
				repeat_delta *= 2;
				break;
			default:
				return false;
		}
	}
	return ret;
}
