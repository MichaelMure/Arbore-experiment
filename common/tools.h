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
 * $Id$
 */

#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include "pf_types.h"

bool is_ip(const char* ip);
std::string stringtok(std::string &in, const char * const delimiters = " \t\n");
std::string pf_addr2string(const pf_addr addr);

// Endian conversion functions
pf_addr nto_pf_addr(pf_addr addr);
pf_addr pf_addr_ton(pf_addr addr);

extern uint64_t htonll(uint64_t number);
extern uint64_t ntohll(uint64_t number);

#endif /* TOOLS_H */
