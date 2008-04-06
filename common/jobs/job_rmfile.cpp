#include "job_rmfile.h"
#include "cache.h"
#include "log.h"

bool JobRmFile::Start()
{
	try
	{
		cache.RmFile(file, sender);
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		log[W_DESYNCH] << "Unable to remove " << file << ": No such file or directory";
		/* TODO: Desynch, DO SOMETHING */
	}
	catch(Cache::DirNotEmpty &e)
	{
		log[W_DESYNCH] << "Unable to remove " << file << ": Dir not empty";
		/* TODO: Desynch, DO SOMETHING */
	}
	return false;
}
