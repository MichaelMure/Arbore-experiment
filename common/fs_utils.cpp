#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include "fs_utils.h"

bool check_filename(std::string f)
{
	if(f.find("/../") != std::string::npos)
		return false;
	if(f.find("../") == 0)
		return false;
	return true;
}

bool check_filemode(mode_t m)
{
	mode_t authorized = S_IFREG | S_IFDIR | S_IRWXU | S_IRWXG | S_IRUSR | S_IXUSR;
	return !((~authorized) & m);
}
