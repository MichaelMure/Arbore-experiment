#ifndef JOB_SEND_CHANGES_H
#define JOB_SEND_CHANGES_H
#include <time.h>
#include "job.h"
#include "pf_types.h"

class JobSendChanges : public Job
{
	pf_id sender;
	time_t last_view;

public:
	JobSendChanges(pf_id _sender, time_t _last_view) : Job(0, REPEAT_NONE), sender(_sender), last_view(_last_view) {}
	~JobSendChanges() {}

	bool Start();
	job_type GetType() const { return JOB_SEND_CHANGES; }
};

#endif /* JOB_SEND_CHANGES_H */
