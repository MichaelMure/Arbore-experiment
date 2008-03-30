#ifndef JOB_NEW_CONNECTION_H
#define JOB_NEW_CONNECTION_H
#include "job.h"
#include "pf_types.h"

class JobNewConnection : public Job, private pf_addr
{
	time_t dt;
public:
	JobNewConnection(pf_addr addr) :
	Job(time(NULL)), pf_addr(addr) { dt = 1; }
	JobNewConnection(const JobNewConnection* j) :
	Job(time(NULL) + j->dt),  pf_addr(pf_addr(*j)), dt(j->dt) {}

	void Start();
	job_type GetType() const {return JOB_NEW_CONNECT; }
	bool IsMe(const pf_addr&);
};

#endif
