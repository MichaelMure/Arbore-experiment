#ifndef JOB_ADVERTISE_FILE_H
#define JOB_ADVERTISE_FILE_H
#include "job.h"
#include "pf_types.h"

class JobAdvertiseFile : public Job
{
	std::string filename;
	pf_id advertise_to;
public:
	JobAdvertiseFile(std::string _filename, pf_id _advertise_to) : Job(0, REPEAT_NONE), filename(_filename), advertise_to(_advertise_to) {}

	bool Start();
	job_type GetType() const { return JOB_ADVERTISE_FILE; }
	std::string GetName() const { return "JobAdvertiseFile"; }
};

#endif /* JOB_ADVERTISE_FILE_H */
