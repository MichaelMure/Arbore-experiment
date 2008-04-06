#include "job_mkfile.h"
#include "cache.h"
#include "log.h"

bool JobMkFile::Start()
{
	try
	{
		cache.MkFile(file, stat, sender);
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		log[W_DESYNCH] << "Unable to create " << file << ": No such file or directory";
		/* XXX: Desync DO SOMETHING */
	}
	catch(Cache::FileAlreadyExists &e)
	{
		log[W_DESYNCH] << "Unable to create " << file << ": File already exists";
		/* XXX: DO SOMETHING */
	}

	return false;
}
