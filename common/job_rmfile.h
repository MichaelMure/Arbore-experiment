#ifndef JOB_RMFILE_H
#define JOB_RMFILE_H
#include "job.h"
#include "pf_file.h"
#include "pf_types.h"

class JobRmFile : public Job
{
	std::string file;
	pf_id sender;
public:
	JobRmFile(std::string _file, pf_id _sender): Job(0, REPEAT_NONE), file(_file), sender(_sender) {}
	~JobRmFile() {}

	bool Start();

	job_type GetType() const { return JOB_RMFILE; }
};

#endif /* JOB_RMFILE_H */
