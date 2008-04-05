#include "job_new_conn_req_base.h"
#include "log.h"
#include "network.h"

bool JobNewConnReqBase::Start()
{
	try
	{
		net.Connect(*this);
		OnSuccess();
	}
	catch(Network::CantConnectTo &e)
	{
		log[W_INFO] << "Unable to connect to " << (pf_addr)*this;
		OnFailure();
	}
	return false;
}
