#include "job_send_ref_file.h"
#include "content_list.h"
#include "packet.h"
#include "peers_list.h"

bool JobSendRefFile::Start()
{
	uint32_t ref = content_list.GetRef(filename);
	FileContent& f = content_list.GetFile(filename);
	off_t offset;
	size_t size;
	f.GetOnDiskContent(offset, size);

	Packet packet(NET_REF_FILE);
	packet.SetArg(NET_REF_FILE_PATH, filename);
	packet.SetArg(NET_REF_FILE_REF, ref);
	packet.SetArg(NET_REF_FILE_OFFSET, offset);
	packet.SetArg(NET_REF_FILE_SIZE, size);
	return false;
}
