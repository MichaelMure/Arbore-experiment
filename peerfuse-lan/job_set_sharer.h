#ifndef JOB_SET_SHARER_H
#define JOB_SET_SHARER_H
#include "job.h"
#include "pf_types.h"
#include "job_types.h"

class JobSetSharer : public Job
{
	std::string filename;
	pf_id sharer;
public:
	JobSetSharer(std::string _filename, pf_id _sharer) : Job(0, REPEAT_NONE), filename(_filename), sharer(_sharer) {}

	bool Start();

	job_type GetType() const { return JOB_SET_SHARER; }
	std::string GetName() const { return "JobSetSharer"; }
};

#endif /* JOB_SET_SHARER_H */
