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

#ifndef PF_FILEBASE_H
#define PF_FILEBASE_H

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "file_perms.h"
#include "pf_types.h"
#include "pf_log.h"

class DirEntry;

class pf_stat : public FilePermissions
{
public:
	off_t size;
	time_t atime;
	time_t mtime;
	time_t ctime;
	time_t meta_mtime;		  /* last meta-data modification */

	pf_stat()
		: size(0),
		atime(0),
		mtime(0),
		ctime(0),
		meta_mtime(0) {}

	bool operator==(const pf_stat& st)
	{
		return (size == st.size &&
			atime == st.atime &&
			mtime == st.mtime &&
			ctime == st.ctime &&
			meta_mtime == st.meta_mtime &&
			pf_mode == st.pf_mode &&
			mode == st.mode &&
			uid == st.uid &&
			gid == st.gid);
	}

};

class FileEntryBase
{
	const std::string name;
	DirEntry* parent;
	IDList sharers;

protected:
	pf_stat stat;
public:

	FileEntryBase(std::string name, pf_stat stat, DirEntry* _parent);
	virtual ~FileEntryBase();

	DirEntry* GetParent() const { return parent; }
	std::string GetName() const { return name; }
	std::string GetFullName() const;

	bool IsChildOf(const FileEntryBase* f) const;

	IDList GetSharers() const { return sharers; }
	void SetSharers(IDList l);
	void AddSharer(pf_id id) { sharers.insert(id); SetSharers(sharers); }

	pf_stat GetAttr() const { return stat; };
	void SetAttr(pf_stat stat, bool force = false);

	/** Load attributes from tree_cfg file. */
	virtual void LoadAttr();

	bool IsRemoved() const { return stat.pf_mode & pf_stat::S_PF_REMOVED; }
	void SetRemoved() { stat.pf_mode |= pf_stat::S_PF_REMOVED; }
	void ClearRemoved() { stat.pf_mode |= pf_stat::S_PF_REMOVED; }
};

template<>
inline Log::flux& Log::flux::operator<< <FileEntryBase*> (FileEntryBase* file)
{
	str += file->GetFullName();
	return *this;
}
#endif						  /* PF_FILEBASE_H */
