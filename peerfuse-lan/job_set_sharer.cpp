#include "job_set_sharer.h"
#include "content_list.h"

bool JobSetSharer::Start()
{
	content_list.SetSharer(filename, sharer);
	return false;
}
