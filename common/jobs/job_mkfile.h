#ifndef JOB_MKFILE_H
#define JOB_MKFILE_H
#include "job.h"
#include "pf_file.h"
#include "pf_types.h"

class JobMkFile : public Job
{
	std::string file;
	pf_stat stat;
	pf_id sender;
public:
	JobMkFile(std::string _file, pf_stat _stat, pf_id _sender): Job(0, REPEAT_NONE), file(_file), stat(_stat), sender(_sender) {}
	~JobMkFile() {}

	bool Start();

	job_type GetType() const { return JOB_MKFILE; }
};

#endif /* JOB_MKFILE_H */
