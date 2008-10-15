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
#include <list>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "xmlrpc.h"
#include "pf_config.h"
#include "peer.h"
#include "cache.h"

#include <peerfuse.h>

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

// Method to give list of knowns peers
class _PeersList : public XmlRpcServerMethod
{
public:
	_PeersList(XmlRpcServer* s) : XmlRpcServerMethod("peers.list", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		int index = 0;
		PeersListBase* plb = xmlrpc.GetPeersListBase();
		if(!plb)
			return;

		IDList list = plb->GetIDList();
		while(list.size())
		{
			result[index++] = TypToStr((uint32_t)(*(list.begin())));
			list.erase(list.begin());
		}
	}
};

// Method to give information about a peer
class _PeersInfo : public XmlRpcServerMethod
{
public:
	_PeersInfo(XmlRpcServer* s) : XmlRpcServerMethod("peers.info", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		// Check we have a valid string
		std::string s_id = params[0];
		if(s_id == "")
			return;

		// Check we have a peerlistbase
		PeersListBase* plb = xmlrpc.GetPeersListBase();
		if(!plb)
		{
			result = false;
			return;
		}

		/* If we are not connected directly to peer, the fd
		 * can be < 0.
		 */
		pf_id pfid = StrToTyp<pf_id>(s_id);
		int fd = plb->GetPeersFD(pfid);

		// Fill info
		result["fd"] = fd;
		result["pf_id"] = s_id;
		result["pf_addr"] = pf_addr2string(plb->GetPeerAddr(pfid));
		std::string state;
		switch (plb->WhatIsThisID(pfid))
		{
			case PeersListBase::IS_ON_NETWORK:
				state = "is_on_network";
				break;
			case PeersListBase::IS_CONNECTED:
				state = "connected";
				break;
			default:
			case PeersListBase::IS_UNKNOWN:
				state = "unknown";
				break;
		}
		result["state"] = state;
	}
};

// Filler for the _BrowserReaddir class
int _BrowserReaddir_filler(void *buf, const char *name, const struct stat *stbuf, off_t off)
{
	((std::list<std::string> *)buf)->push_back(std::string(name));
	return 0;
}

// Method to get file from directory
class _BrowserReaddir : public XmlRpcServerMethod
{
public:
	_BrowserReaddir(XmlRpcServer* s) : XmlRpcServerMethod("browser.readdir", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		// Check we have a valid string
		std::string s_path = params[0];
		if(s_path == "")
			s_path = "/";

		std::list<std::string> data;
		try
		{
			cache.FillReadDir(s_path.c_str(), (void*)&data, _BrowserReaddir_filler, 0, NULL);
		}
		catch (...)
		{
			// TODO return an XMLRPC Exception
			result = false;
			return;
		}

		int index = 0;
		while (data.size())
		{
			result[index++] = *(data.begin());
			data.pop_front();
		}

	}
};

// Method to get attr from a file
class _BrowserGetattr : public XmlRpcServerMethod
{
public:
	_BrowserGetattr(XmlRpcServer* s) : XmlRpcServerMethod("browser.getattr", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		// Check we have a valid string
		std::string s_path = params[0];
		if(s_path == "")
			s_path = "/";

		pf_stat stat;
		try
		{
			stat = cache.GetAttr(s_path);
		}
		catch (...)
		{
			// TODO return an XMLRPC Exception
			result = false;
			return;
		}

		result["pf_mode"]		= (int)stat.pf_mode;
		result["mode"]			= (int)stat.mode;
		result["uid"]			= (int)stat.uid;
		result["gid"]			= (int)stat.gid;
		result["size"]			= (int)stat.size;
		result["atime"]			= (int)stat.atime;
		result["mtime"]			= (int)stat.mtime;
		result["ctime"]			= (int)stat.ctime;
		result["meta_mtime"]	= (int)stat.meta_mtime;
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
	methods.push_back(new _PeersInfo(&s));
	methods.push_back(new _BrowserReaddir(&s));
	methods.push_back(new _BrowserGetattr(&s));

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

