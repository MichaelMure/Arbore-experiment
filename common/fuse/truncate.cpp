#define FUSE_USE_VERSION 26
#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include "cache.h"
#include "log.h"

int pf_truncate(const char* path, off_t offset)
{
	try
	{
		cache.Truncate(std::string(path), offset);
	}
	catch(...)
	{
		log[W_ERR] << "Failed to truncate to " << path;
		return -ENOENT;
	}
	return 0;
}
