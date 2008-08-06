/*
 * Copyright(C) 2008 Mathieu Virbel <tito@bankiz.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * 
 */

#include <map>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "xmlrpc.h"
#include "pf_config.h"
#include "peer.h"

#ifdef PF_NET
#include <pfnet.h>
#elif PF_LAN
#include <pflan.h>
#endif

using namespace XmlRpc;

XmlRpcThread xmlrpc;

// Method to give version of peerfuse
class _PfVersion : public XmlRpcServerMethod
{
public:
	_PfVersion(XmlRpcServer* s) : XmlRpcServerMethod("pf.version", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		result = PEERFUSE_VERSION;
	}
};

// Method to give all informations about peerfuse
class _PfInfos : public XmlRpcServerMethod
{
public:
	_PfInfos(XmlRpcServer* s) : XmlRpcServerMethod("pf.infos", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		result["peerfuse.version"] = PEERFUSE_VERSION;
		result["peerfuse.version.name"] = PEERFUSE_VERSION_NAME;
		result["peerfuse.version.major"] = PEERFUSE_VERSION_MAJOR;
		result["peerfuse.version.minor"] = PEERFUSE_VERSION_MINOR;
		result["peerfuse.version.proto"] = PEERFUSE_VERSION_PROTO;
		result["peerfuse.build"] = "Build " __DATE__ " " __TIME__;
		result["xmlrpc.version"] = XMLRPC_VERSION;
	}
};

// Method to give informations about peers
class _PeersList : public XmlRpcServerMethod
{
public:
	_PeersList(XmlRpcServer* s) : XmlRpcServerMethod("peers.list", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		PeersListBase* l;
		std::vector<Peer*>::iterator it;
		int index;
		std::ostringstream oss;

		l = xmlrpc.GetPeersListBase();
		if(!l)
			return;

		it = l->begin();
		for(index = 0; it != l->end(); it++, index++)
		{
			oss << (uint32_t)(**it).GetID();
			result[index] = oss.str();
			oss.str("");
		}
	}
};

XmlRpcThread::XmlRpcThread()
{
	port = 1772;
	started = false;
	network = NULL;
	peers_list = NULL;
}

XmlRpcThread::~XmlRpcThread()
{
	started = false;
}

void XmlRpcThread::OnStart()
{
	// Read from config
	port = conf.GetSection("xmlrpc")->GetItem("port")->Integer();
	if(port <= 0)
		return;

	// Instances methods
	methods.push_back(new _PfVersion(&s));
	methods.push_back(new _PfInfos(&s));
	methods.push_back(new _PeersList(&s));

	// Bind
	started = true;
	s.bindAndListen(port);
}

void XmlRpcThread::OnStop()
{
	if(!started)
		return;

	s.exit();
	s.shutdown();

	/**
	 * XXX Search in XmlRpcServer how methods are clean
	std::list<XmlRpcServerMethod*>::iterator it = methods.begin();
	while(it != methods.end())
	{
		delete (*it);
	}
	**/
}

void XmlRpcThread::Loop()
{
	if(!started)
		return;

	s.work(10);
}

