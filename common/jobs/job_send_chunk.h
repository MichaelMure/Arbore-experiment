#ifndef JOB_SEND_CHUNK_H
#define JOB_SEND_CHUNK_H
#include "job.h"
#include "job_types.h"
#include "pf_types.h"

class JobSendChunk : public Job
{
	uint32_t ref;
	pf_id sendto;
	off_t offset;
	size_t size;
public:
	JobSendChunk(uint32_t _ref, pf_id _sendto, off_t _offset, size_t _size) : Job(2, REPEAT_PERIODIC), ref(_ref), sendto(_sendto), offset(_offset), size(_size) {}

	bool Start();

	job_type GetType() const { return JOB_SEND_CHUNK; }
	std::string GetName() const { return "JobSendChunk"; }
};

#endif /* JOB_SEND_CHUNK_H */
