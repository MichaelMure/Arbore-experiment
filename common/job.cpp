#include <time.h>
#include "job.h"

bool Job::DoStart()
{
	bool ret = Start();

	if(ret)
	{
		// Update the start-time
		switch(repeat_type)
		{
		case REPEAT_NONE:
			return false;
		case REPEAT_PERIODIC:
			start_time = time(NULL) + repeat_delta;
			break;
		case REPEAT_LESS_AND_LESS:
			start_time += time(NULL) + repeat_delta;
			repeat_delta *= 2;
			break;
		default:
			return false;
		}
	}
	return ret;
}
