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

#ifndef FILE_ENTRY_H
#define FILE_ENTRY_H

#include <set>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <util/key.h>
#include <util/pf_types.h>
#include <util/pf_log.h>
#include "file_perms.h"

class DirEntry;
class FileEntry;

/** This class extend FilePermissions to provide a full set of metadata
 */
class pf_stat : public FilePermissions
{
public:
	off_t size;		/** size of the file */
	time_t atime;		/** access time */
	time_t mtime;		/** modification time */
	time_t ctime;		/** creation time */
	time_t meta_mtime;	/** last meta-data modification */

	pf_stat();

	bool operator==(const pf_stat& st);
};

struct CompFiles
{
	bool operator() (const FileEntry* f1, const FileEntry* f2) const;
};

typedef std::set<FileEntry*, CompFiles> FileList;

/**
 * Abstraction of a single file.
 */
class FileEntry
{
	const std::string name; /** Name of the file */
	DirEntry* parent; /** Parent directory */
	KeyList sharers; /** key of nodes that share this file */
	Key key; /** probably unique key of the file for the DHT */

protected:
	pf_stat stat; /** set of attributes */
public:

	/**
	 * Constructor of FileEntry.
	 */
	FileEntry(std::string name, pf_stat stat, DirEntry* _parent);
	virtual ~FileEntry() {};

	/**
	 * @return the parent directory.
	 */
	DirEntry* GetParent() const;

	/**
	 * @return the name of the file.
	 */
	std::string GetName() const;

	/**
	 * @return the full path of the file (e.g: path/to/file).
	 */
	std::string GetFullName() const;

	/**
	 * I'm not sure here: currently, this return the key corresponding of the name
	 * of the file (computed in the constructor). Shouldn't this be the key of the full path
	 * as seems to indicate the name of the function ?
	 */
	Key getPathSerial() const;

	/** Indicate if a the file if a child of the given FileEntry (DirEntry inherit from FileEntry).
	 * @param f the FileEntry to chek
	 */
	bool IsChildOf(const FileEntry* f) const;

	/**
	 * @return the list of sharer of the file.
	 */
	KeyList GetSharers() const;

	/**
	 * Overwrite the list of sharers of the file and update the session config.
	 */
	void SetSharers(KeyList l);

	/**
	 * Add a sharer to the sharers list and update the session config.
	 */
	void AddSharer(Key id);

	/**
	 * @return the attributes of the file.
	 */
	pf_stat GetAttr() const;

	/**
	 * Update the attributes of the file.
	 * @param stat the new set of attributes
	 * @param force if set, the session config is not checked and is overwrited.
	 */
	void SetAttr(pf_stat stat, bool force = false);

	/**
	 * Load attributes from tree_cfg file.
	 */
	void LoadAttr();

	/**
	 * @return if the file is marked as removed.
	 */
	bool IsRemoved() const;

	/**
	 * Set the file as removed.
	 */
	void SetRemoved();

	/**
	 * Unset the file as removed.
	 */
	void ClearRemoved();
};

template<>
inline Log::flux& Log::flux::operator<< <FileEntry*> (FileEntry* file)
{
	str += file->GetFullName();
	return *this;
}
#endif						  /* FILE_ENTRY_H */
