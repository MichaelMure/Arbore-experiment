#include "job_set_shared_part.h"
#include "content_list.h"
#include "file_content.h"

bool JobSetSharedPart::Start()
{
	FileContent& f = content_list.GetFile(filename);
	f.SetSharer(sharer, offset, size);
	return false;
}
