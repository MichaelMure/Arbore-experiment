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

	SslSsl ssl(listen_fd);

	struct sockaddr_in sa_serv;
	memset (&sa_serv, '\0', sizeof(sa_serv));
	sa_serv.sin_family      = AF_INET;
	sa_serv.sin_addr.s_addr = INADDR_ANY;
	sa_serv.sin_port        = htons (8000);
}
