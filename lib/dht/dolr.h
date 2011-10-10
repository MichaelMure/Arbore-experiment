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
 */

#ifndef _DOLR_H
#define _DOLR_H

#include <map>
#include <set>
#include <util/key.h>

/** DOLR : Distributed Object Location and Routing
 *
 * DOLR is used to maintain a mapping between an object and its' owner(s)
 * NOT USED AT THE MOMENT
 */
class DOLR
{
	typedef std::map<Key, std::set<Key> > TableType;
	TableType table;

public:

	DOLR();
	~DOLR();
  /** Add the object's key if it does not exist in the table, and the owner in the table*/
	void published(Key key, Key owner);
  /** Remove the owner of the list of key owners */
	void unpublised(Key key, Key owner);
};

#endif /* _DOLR_H */
