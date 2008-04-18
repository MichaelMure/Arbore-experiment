#ifndef FILE_CONTENT_H
#define FILE_CONTENT_H
#include "file_content_base.h"

class FileContent : public FileContentBase
{
	FileChunk& operator=(const FileChunk &other);
public:
	FileContent(std::string path) : FileContentBase(path) {}
	~FileContent() {}
};

#endif /* FILE_CONTENT_H */

