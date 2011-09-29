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

#ifndef DTIME_H
#define DTIME_H

#include <sys/time.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   ** dtime:
   **  returns the time of day in double format with microsecond precision
   */
    extern double dtime ();

  /**
   ** dalarm:
   **  generates a SIGALRM signal in #time# seconds
   */
    extern void dalarm (double time);

  /**
   ** dalarm:
   **  sleeps for #time# seconds
   */
    extern void dsleep (double time);

  /**
   ** dtotv:
   **  returns the struct timeval representation of double #d#
   */
    extern struct timeval dtotv (double d);

  /**
   ** tvtod:
   **  returns the double representation of timeval #tv#
   */
    extern double tvtod (struct timeval tv);

#ifdef __cplusplus
}
#endif

#endif				/* DTIME_H */
