#ifndef JOB_SEND_REF_FILE_H
#define JOB_SEND_REF_FILE_H
#include <string.h>
#include "job.h"
#include "pf_types.h"

class JobSendRefFile : public Job
{
	std::string filename;
	pf_id sendto;
public:
	JobSendRefFile(std::string _filename, pf_id _sendto) : Job(0, REPEAT_NONE), filename(_filename), sendto(_sendto) {}

	bool Start();

	job_type GetType() const { return JOB_SEND_REF_FILE; }
	std::string GetName() const { return "JobSendRefFile"; }
};

#endif /* JOB_SEND_REF_FILE_H */
