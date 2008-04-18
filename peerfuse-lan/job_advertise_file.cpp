#include "job_advertise_file.h"
#include "content_list.h"
#include "packet.h"
#include "peers_list.h"
#include "net_proto.h"

bool JobAdvertiseFile::Start()
{
	if(content_list.HaveFile(filename))
	{
		Packet packet(NET_I_HAVE_FILE);
		packet.SetArg(NET_I_HAVE_FILE_PATH, filename);
		peers_list.SendMsg(advertise_to, packet);
	}
	return false;
}
