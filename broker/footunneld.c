#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <argp.h>

#define BUFLEN 512

const char *argp_program_version = "footunneld 0.1";
const char *argp_program_bug_address = "<mail@justus-beyer.de>";

static char doc[] = "footunneld -- a simple Foo-over-UDP tunnel broker proof of concept";
static char args_doc[] = "<tunnel device> <secret>";

static struct argp_option options[] = {
        {"broker-port", 'p', "port",    0,  "Server/Broker UDP port" },
        {"verbose",     'v', 0,         0,  "Produce verbose output" },
        { 0 }
};

struct arguments
{
	char *tunnel_dev;
	char *secret;
	unsigned int broker_port;
	int verbose;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key)
	{
	case 'p': // broker port
		arguments->broker_port = atoi(arg);
		break;

	case 'v': // verbose
		arguments->verbose = 1;
		break;

	case ARGP_KEY_ARG:
		switch (state->arg_num)
		{
		case 0:         arguments->tunnel_dev = arg; break;
		case 1:         arguments->secret = arg; break;
		default:        argp_usage(state);
		}
		break;

	case ARGP_KEY_END:
		if (state->arg_num < 2)
			argp_usage(state);
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

void diep(char *s)
{
	perror(s);
	exit(1);
}

void open_socket(unsigned int broker_port, int *sock)
{
	struct sockaddr_in si_me;

	if ((*sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("Failed to obtain socket");

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(broker_port);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(*sock, (struct sockaddr*)&si_me, sizeof(si_me))==-1)
		diep("Failed to bind socket to recv port.");
}

int update_tunnel_peer_ip(const char* tunnel_dev, struct sockaddr_in *si_peer)
{
	const int MAXLEN = 128;
	char update_cmd[MAXLEN];
	snprintf(update_cmd, MAXLEN, "ip tunnel change %s remote %s\n",
		tunnel_dev, inet_ntoa(si_peer->sin_addr));
	return system(update_cmd);
}

int main(int argc, char **argv)
{
	struct arguments arguments;

	// Default values
	arguments.broker_port = 42425;
	arguments.verbose     = 0;

        // Parse arguments
        argp_parse (&argp, argc, argv, 0, 0, &arguments);

	struct sockaddr_in si_peer, si_peer_lastseen;
	int socket, addr_len=sizeof(si_peer);
	char buf[BUFLEN];

	open_socket(arguments.broker_port, &socket);
	printf("Waiting for heartbeats on UDP port %d ...\n", arguments.broker_port);

	while(recvfrom(socket, buf, BUFLEN, 0, (struct sockaddr*)&si_peer, &addr_len) != -1)
	{
		// Check if the packet contains the right secret
		if (strncmp(buf, arguments.secret, BUFLEN) != 0)
			; //continue;

		// Check if the peer's ip has changed
		if (si_peer.sin_addr.s_addr != si_peer_lastseen.sin_addr.s_addr)
		{
			// Peer IP changed -> change tunnel config
			printf("Peer for tunnel %s is now at %s:%u\n",
				arguments.tunnel_dev,
				inet_ntoa(si_peer.sin_addr),
				si_peer.sin_port);
			update_tunnel_peer_ip(arguments.tunnel_dev, &si_peer);
			si_peer_lastseen = si_peer;
		}
		else if (arguments.verbose)
		{
			printf("Heartbeat received from peer %s:%u for tunnel %s.\n",
				inet_ntoa(si_peer.sin_addr),
				si_peer.sin_port,
				arguments.tunnel_dev);
		}

		// TODO: We're not looking at the port in this PoC!
	}

	close(socket);
	return 0;
}
