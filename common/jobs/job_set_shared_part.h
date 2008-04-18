#ifndef JOB_SET_SHARED_PART_H
#define JOB_SET_SHARED_PART_H
#include "job.h"
#include "pf_types.h"
#include "job_types.h"

class JobSetSharedPart : public Job
{
	std::string filename;
	pf_id sharer;
	off_t offset;
	size_t size;
public:
	JobSetSharedPart(std::string _filename, pf_id _sharer, off_t _offset, size_t _size) :
		Job(0, REPEAT_NONE), filename(_filename), sharer(_sharer), offset(_offset), size(_size) {}

	bool Start();
	job_type GetType() const { return JOB_SET_SHARED_PART; }
	std::string GetName() const { return "JobSetSharedPart"; }
};

#endif /* JOB_SET_SHARED_PART_H */
