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

#include <stdio.h>
#include <string.h>		/* memset */
#include <sys/time.h>		/* struct timeval, gettimeofday */
#include <signal.h>		/* siginterrupt, SIGALRM */
#include "time.h"		/* function headers */

double time::dtime (void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (tvtod (tv));
}

void time::dalarm (double time)
{
    struct itimerval it;
    memset (&it, 0, sizeof (struct itimerval));
    it.it_value = dtotv (time);
    siginterrupt (SIGALRM, 1);
    setitimer (ITIMER_REAL, &it, NULL);
}

void time::dsleep (double time)
{
    struct timeval tv;
    tv = dtotv (time);
    select (0, NULL, NULL, NULL, &tv);
}

struct timeval time::dtotv (double d)
{
    struct timeval tv;
    tv.tv_sec = (long) d;
    tv.tv_usec = (long) ((d - (double) tv.tv_sec) * 1000000.0);
    return (tv);
}

double time::tvtod (struct timeval tv)
{
    return ((double)tv.tv_sec + ((double) tv.tv_usec / 1000000.0));
}
