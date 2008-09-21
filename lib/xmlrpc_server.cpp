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

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "pf_config.h"
#include "pf_log.h"
#include "xmlrpc_server.h"

bool PfXmlRpcServer::bindAndListen(int port, int backlog)
{
	int sflag = 1;
	int fd = -1;
	const char *bind_ip = NULL;

	// Get bind ip from config
	bind_ip = conf.GetSection("xmlrpc")->GetItem("bind")->String().c_str();
	pf_log[W_DEBUG] << "XmlRpc: bind to " << bind_ip << ":" << port;

	// Create socket
	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(fd < 0)
	{
		pf_log[W_ERR] << "XmlRpc: unable to create socket !";
		return false;
	}

	this->setfd(fd);

	// Set non-blocking
	if (fcntl(fd, F_SETFL, O_NONBLOCK) != 0)
	{
		pf_log[W_ERR] << "XmlRpc: unable to set socket as non-blocking !";
		this->close();
		return false;
	}

	// Allow this port to be re-bound immediately so server re-starts are not delayed
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&sflag, sizeof(sflag)) != 0)
	{
		pf_log[W_ERR] << "XmlRpc: unable to set socket as reuseaddr !";
		this->close();
		return false;
	}

	// Bind to a specified ip/port
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr(bind_ip);
	saddr.sin_port = htons((u_short) port);

	if(bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
	{
		pf_log[W_ERR] << "XmlRpc: unable to bind socket !";
		this->close();
		return false;
	}

	// Listen mode
	if(listen(fd, backlog) != 0)
	{
		pf_log[W_ERR] << "XmlRpc: unable to listen socket !";
		this->close();
		return false;
	}

	// Notify that we can work :)=
	_disp.addSource(this, XmlRpc::XmlRpcDispatch::ReadableEvent);

	return true;
}

