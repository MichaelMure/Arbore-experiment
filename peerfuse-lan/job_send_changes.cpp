#include <time.h>
#include "job_send_changes.h"
#include "cache.h"

bool JobSendChanges::Start()
{
	cache.SendChanges(sender, last_view);
	return false;
}
