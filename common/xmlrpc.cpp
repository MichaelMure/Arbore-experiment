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
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "xmlrpc.h"
#include "pf_config.h"

PfXmlRpcServer xmlrpc;

class Hello : public XmlRpc::XmlRpcServerMethod
{
public:
	Hello(XmlRpc::XmlRpcServer* s) : XmlRpc::XmlRpcServerMethod("Hello", s) {}
	void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result)
	{
		result = "Hello";
	}
};


PfXmlRpcServer::PfXmlRpcServer()
{
	port = 1772;
	started = false;
}

PfXmlRpcServer::~PfXmlRpcServer()
{
	started = false;
}

void PfXmlRpcServer::OnStart()
{
	// Read from config
	port = conf.GetSection("xmlrpc")->GetItem("port")->Integer();
	if(port <= 0)
		return;

	// Instances methods
	new Hello(&s);

	// Bind
	started = true;
	s.bindAndListen(port);
}

void PfXmlRpcServer::OnStop()
{
	if(!started)
		return;

	s.exit();
	s.shutdown();
}

void PfXmlRpcServer::Loop()
{
	if(!started)
		return;

	s.work(10);
}

/**
class Hello : public XmlRpcServerMethod
{
public:
  Hello(XmlRpcServer* s) : XmlRpcServerMethod("Hello", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
    result = "Hello";
  }

} hello(&s);    // This constructor registers the method with the server


// The port to use
const int PORT = 8080;

int main(int argc, char* argv[]) {

  // Create the server socket on the specified port
  s.bindAndListen(PORT);

  // Wait for requests and process indefinitely (Ctrl-C to exit)
  s.work(-1.0);

  return 0;
}
**/
