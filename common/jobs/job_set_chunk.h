#ifndef JOB_SET_CHUNK_H
#define JOB_SET_CHUNK_H
#include "job.h"
#include "job_types.h"
#include "file_content.h"

class JobSetChunk : public Job
{
	std::string filename;
	FileChunk chunk;
public:
	JobSetChunk(std::string _filename, FileChunk _chunk) : Job(0, REPEAT_NONE), filename(_filename), chunk(_chunk) {}

	bool Start();

	job_type GetType() const { return JOB_SET_CHUNK; }
	std::string GetName() const { return "JobSetChunk"; }
};

#endif /* JOB_SET_CHUNK_H */
