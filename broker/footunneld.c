#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFLEN 512
#define PORT 42425

void diep(char *s)
{
	perror(s);
	exit(1);
}



int main(void)
{
	struct sockaddr_in si_me, si_other;
	int s, i, slen=sizeof(si_other);
	char buf[BUFLEN];

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me))==-1)
		diep("bind");

	while(recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen) != -1)
	{
		if (strncmp(buf, "identifier", BUFLEN) != 0)
			; //continue;

		char update_cmd[BUFLEN];
		snprintf(update_cmd, BUFLEN, "ip tunnel change fou0 remote %s\n", inet_ntoa(si_other.sin_addr));
		printf(update_cmd);
		int res = system(update_cmd);
		printf("Result: %d\n", res);
	}

	close(s);
	return 0;
}
