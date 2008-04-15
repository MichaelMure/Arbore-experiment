#define FUSE_USE_VERSION 26
#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include "cache.h"
#include "log.h"

int pf_read(const char *path, char *buf, size_t size,
			off_t offset, struct fuse_file_info *fi)
{
	try
	{
		int bytes_read = cache.Read(std::string(path), buf, size, offset);
		return bytes_read;
	}
	catch(...)
	{
		log[W_ERR] << "Failed to write to " << path;
		return -ENOENT;
	}
	return 0;
}

