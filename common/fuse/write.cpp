#define FUSE_USE_VERSION 26
#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include "cache.h"
#include "log.h"

int pf_write(const char *path, const char *buf, size_t size,
			off_t offset, struct fuse_file_info *fi)
{
	try
	{
		cache.Write(std::string(path), buf, size, offset);
		return (int)size;
	}
	catch(...)
	{
		log[W_ERR] << "Failed to write to " << path;
		return -ENOENT;
	}
	return 0;
}

