#ifndef CONTENT_LIST_H
#define CONTENT_LIST_H

#include <string.h>
#include <map>
#include "pf_thread.h"
#include "file_content.h"

class ContentList : public Thread, std::map<std::string, FileContent>
{
public:
	/** Returns the content of the file we already have in memory
	 * @param path path to the file
	 * @return the file content
	 */
	FileContent& GetFile(std::string path);

	/** Tells if we have a FileContent in our list
	 */
	bool HaveFile(std::string path);
};

extern ContentList content_list;

#endif
