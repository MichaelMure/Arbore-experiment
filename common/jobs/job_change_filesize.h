#ifndef JOB_CHANGE_FILESIZE_H
#define JOB_CHANGE_FILESIZE_H
#include <string.h>
#include "job.h"

class JobChangeFileSize : public Job
{
	std::string path;
	size_t size;
public:
	JobChangeFileSize(std::string _path, size_t _size) : Job(0, REPEAT_NONE), path(_path), size(_size) {}

	bool Start();
	job_type GetType() const { return JOB_CHANGE_FILESIZE; }
	std::string GetName() const { return "JobChangeFileSize"; }
};

#endif /* JOB_CHANGE_FILESIZE_H */
