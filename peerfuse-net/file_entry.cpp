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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#include "file_entry.h"
#include "session_config.h"

bool CompFiles::operator() (const FileEntry* f1, const FileEntry* f2) const
{
	if(f1->IsChildOf(f2))
		return false;
	else if(f2->IsChildOf(f1))
		return true;
	else
		return f1 < f2;
}

FileEntry::FileEntry(std::string _name, pf_stat _stat, DirEntry* parent)
			: FileEntryBase(_name, _stat, parent),
			path_serial(0u)
{
	/* Calculate serial */
	for(std::string::iterator c = _name.begin(); c != _name.end(); ++c)
		path_serial += (path_serial << 3) + (unsigned char)*c;
}

void FileEntry::LoadAttr()
{
	FileEntryBase::LoadAttr();

	std::string cfg_val_s;
	if(tree_cfg.Get(GetFullName() + "#sharers", cfg_val_s))
	{
		IDList idlist;
		std::string id;
		while((id = stringtok(cfg_val_s, ",")).empty() == false)
			idlist.insert(StrToTyp<uint32_t>(id));
		SetSharers(idlist);
	}

}
