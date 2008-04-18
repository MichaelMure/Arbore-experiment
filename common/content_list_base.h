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

#ifndef CONTENT_LIST_BASE_H
#define CONTENT_LIST_BASE_H

#include <string.h>
#include <map>
#include "mutex.h"
#include "pf_thread.h"
#include "file_content.h"

class ContentListBase : public Thread, private Mutex, private std::map<std::string, FileContent>
{
	// Map the ref i'll use to SEND file
	std::map<uint32_t, std::string> my_refs;

protected:
	void Loop();
	void OnStop();

public:
	virtual ~ContentListBase();

	/** Returns the content of the file we already have in memory
	 * @param path path to the file
	 * @return the file content
	 */
	FileContent& GetFile(std::string path);

	/** Remove a file from the list. It's content is not flushed to the disk
	 * @param path path to the file
	 */
	void RemoveFile(std::string path);

	uint32_t GetRef(std::string filename);
};
#endif						  /* CONTENT_LIST_BASE_H */
