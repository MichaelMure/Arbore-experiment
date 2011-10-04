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

#ifndef TIME_H
#define TIME_H

#include <sys/time.h>

class time {

public:
  /** @return the time of day in double format with microsecond precision*/
  static double dtime (void);

  /** Generates a SIGALRM signal after time seconds.*/
  static void dalarm (double time);

  /** Sleeps for time seconds */
  static void dsleep (double time);

  /** @return the struct timeval representation of double d */
  static struct timeval dtotv (double d);

  /** @return the double representation of timeval tv */
  static double tvtod (struct timeval tv);

};

#endif        /* TIME_H */
