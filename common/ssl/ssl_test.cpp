#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pf_ssl_ssl.h"

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
}

