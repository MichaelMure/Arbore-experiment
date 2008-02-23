/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
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
 * $Id$
 */

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pf_ssl_ssl.h"

#define TEST_CLIENT
//#define TEST_SERVER

#ifdef TEST_CLIENT
int main(int argc, char* argv[])
{
	SslSsl ssl;

	int sd = socket (AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in sa;
	memset (&sa, '\0', sizeof(sa));
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = inet_addr ("127.0.0.1");
	sa.sin_port        = htons     (8000);
	
	connect(sd, (struct sockaddr*) &sa, sizeof(sa));
	ssl.Connect(sd);
	ConnectionSsl* conn = ssl.GetConnection(sd);
	conn->Write("Hello", sizeof("Hello"));

	char buf[128];
	memset(buf, 0, sizeof(buf));
	conn->Read(buf, sizeof(buf));
	printf("%s\n", buf);
}
#endif

#ifdef TEST_SERVER
int main(int argc, char* argv[])
{
	int listen_fd = socket (AF_INET, SOCK_STREAM, 0);
	if(listen_fd == -1)
	{
		printf("Socket creatio failed\n");
		exit(EXIT_FAILURE);
	}

	SslSsl ssl;

	struct sockaddr_in sa_cli;
	memset (&sa_cli, '\0', sizeof(sa_cli));
	struct sockaddr_in sa_serv;
	memset (&sa_serv, '\0', sizeof(sa_serv));
	sa_serv.sin_family      = AF_INET;
	sa_serv.sin_addr.s_addr = INADDR_ANY;
	sa_serv.sin_port        = htons (8000);

	if(bind(listen_fd, (struct sockaddr*) &sa_serv, sizeof (sa_serv)) == -1)
	{
		printf("Unable to bind\n");
		exit(EXIT_FAILURE);
	}

	if(listen (listen_fd, 5) == -1)
	{
		printf("Unable to receive\n");
		exit(EXIT_FAILURE);
	}
	
	socklen_t client_len = sizeof(sa_cli);
	int cli_fd;
	if((cli_fd = accept (listen_fd, (struct sockaddr*) &sa_cli, &client_len)) == -1)
	{
		printf("Unable to accept\n");
		exit(EXIT_FAILURE);
	}

	close (listen_fd);

	ssl.HandShake(cli_fd);
	ConnectionSsl* conn = ssl.GetConnection(cli_fd);
	conn->Write("Hello", sizeof("Hello"));

	char buf[128];
	memset(buf, 0, sizeof(buf));
	conn->Read(buf, sizeof(buf));
	printf("%s\n", buf);
}
#endif

