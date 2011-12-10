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

#include <stdlib.h>
#include <string>
#include <iostream>

#include <net/pf_addr.h>
#include <util/pf_log.h>

int main(int argc, char** argv)
{
  pf_addr a = pf_addr((std::string) "192.168.0.1");
  pf_log[W_INFO] << "192.168.0.1:  " << a.GetStr();

  pf_addr b = pf_addr((std::string) "192.168.0.1", 7889);
  pf_log[W_INFO] << "192.168.0.1:7889:  " << b.GetStr();

  pf_addr c = pf_addr((std::string) "2a01:e35:8aca:7e80:218:f3ff:fef9:c374");
  pf_log[W_INFO] << "2a01:e35:8aca:7e80:218:f3ff:fef9:c374  " << c.GetStr();

  pf_addr d = pf_addr((std::string) "2a01:e35:8aca:7e80:218:f3ff:fef9:c374", 1234);
  pf_log[W_INFO] << "[2a01:e35:8aca:7e80:218:f3ff:fef9:c374]:1234  " << d.GetStr();

  pf_log[W_INFO] << " ";

  char buf[pf_addr::size+1];
  char *p = (char*) &buf;
  b.dump(p);
  buf[pf_addr::size] = '\0';

  pf_addr e = pf_addr(p);
  pf_log[W_INFO] << "" << e.GetStr();

  pf_log[W_INFO] << " ";
  pf_addr f = pf_addr((std::string) "192.168.0.1", 1324);
  pf_log[W_INFO] << "192.168.0.1:  " << f.GetStr();

  pf_addr g = pf_addr((std::string) "2a01:e35:8aca:7e80:218:f3ff:fef9:c374", 1234);
  pf_log[W_INFO] << "2a01:e35:8aca:7e80:218:f3ff:fef9:c374:  " << g.GetStr();

  pf_addr h = pf_addr((std::string) "google.com", 1234);
  pf_log[W_INFO] << "google.com:  " << g.GetStr();

  pf_addr i = pf_addr((std::string) "pellelatarte.fr", 1234);
  pf_log[W_INFO] << "pellelatarte.fr:  " << g.GetStr();

  pf_addr j = pf_addr((std::string) "::ffff:192.0.2.128", 1234);
  pf_log[W_INFO] << "::ffff:192.0.2.128:  " << j.GetStr();


	return 0;
}

