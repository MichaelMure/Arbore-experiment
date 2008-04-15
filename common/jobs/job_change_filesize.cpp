#include "job_change_filesize.h"
#include "cache.h"
#include "log.h"

bool JobChangeFileSize::Start()
{
	pf_stat stat = cache.GetAttr(path);
	stat.size = size;
	cache.SetAttr(path, stat);

	/* Todo: send to network */
	return false;
}
