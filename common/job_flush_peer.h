#ifndef JOB_FLUSH_PEER_H
#define JOB_FLUSH_PEER_H
#include "job.h"
#include "job_types.h"
#include "pf_types.h"

class JobFlushPeer : public Job
{
	pf_id id;
public:
	JobFlushPeer(pf_id _id) : Job(0, REPEAT_NONE), id(_id) {};
	~JobFlushPeer() {};

	bool Start();
	job_type GetType() const { return JOB_FLUSH_PEER; }
};

#endif /* JOB_FLUSH_PEER_H */
