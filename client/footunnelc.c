#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 42425
#define SRCPORT 42425

void diep(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{
	struct sockaddr_in addr, srcaddr;
	int s;

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");

	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	memset((char *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr("130.149.222.73");

	memset(&srcaddr, 0, sizeof(srcaddr));
	srcaddr.sin_family = AF_INET;
	srcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srcaddr.sin_port = htons(SRCPORT);

	if (bind(s, (struct sockaddr*)&srcaddr, sizeof(srcaddr)) < 0)
		diep("bind");

	while (1)
	{
		char msg[] = "identifier";
		if (sendto(s, msg, sizeof(msg), 0, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		{
			diep("Failed to send");
		}
		printf(".");
		sleep(10);
	}

	close(s);
	return 0;
}
