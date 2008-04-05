#ifndef JOB_NEW_CONN_REQ_BASE_H
#define JOB_NEW_CONN_REQ_BASE_H
#include "job.h"
#include "pf_types.h"

class JobNewConnReqBase : public Job, private pf_addr
{
public:
	JobNewConnReqBase(pf_addr addr) : Job(time(NULL), REPEAT_NONE), pf_addr(addr) {}

	bool Start();
	job_type GetType() const {return JOB_NEW_CONN_REQ; }
	virtual void OnSuccess() = 0;
	virtual void OnFailure() = 0;
	pf_addr GetAddr() const { return (pf_addr)*this; }
};
#endif /* NEW_CONN_REQ_BASE_H */
