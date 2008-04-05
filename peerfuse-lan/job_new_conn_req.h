#ifndef JOB_NEW_CONN_REQ_H
#define JOB_NEW_CONN_REQ_H
#include "job_new_conn_req_base.h"
#include "pf_types.h"

class JobNewConnReq: public JobNewConnReqBase
{
	id_t requested_by;
public:
	JobNewConnReq(::pf_addr addr, id_t _requested_by) : JobNewConnReqBase(addr),  requested_by(_requested_by){}

	void OnSuccess();
	void OnFailure();
};
#endif /* JOB_NEW_CONN_REQ_H */
