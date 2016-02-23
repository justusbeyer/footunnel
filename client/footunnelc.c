#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <argp.h>

const char *argp_program_version = "footunnelc 0.1";
const char *argp_program_bug_address = "<mail@justus-beyer.de>";

static char doc[] = "footunnelc -- a Foo-over-UDP tunnel heartbeat client";
static char args_doc[] = "<broker ip> <secret>";

static struct argp_option options[] = {
	{"broker-port", 'p', "port",	0,  "Server/Broker UDP port" },
	{"local-port",  'l', "port",	0,  "Local UDP port to use for outgoing heartbeats" },
	{"interval",	'i', "seconds",	0,  "Interval of heartbeat messages in seconds" },
	{"verbose",     'v', 0,		0,  "Produce verbose output" },
	{ 0 }
};

struct arguments
{
	char *broker_ip;
	char *secret;
	int broker_port;
	int local_port;
	int interval;
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

	case 'l': // local udp port
		arguments->local_port = atoi(arg);
		break;

	case 'i': // interval
		arguments->interval = atoi(arg);
		break;

	case 'v': // verbose
		arguments->verbose = 1;
		break;

	case ARGP_KEY_ARG:
		switch (state->arg_num)
		{
		case 0:		arguments->broker_ip = arg; break;
		case 1:		arguments->secret = arg; break;
		default:	argp_usage(state);
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


void open_socket(
	const char *broker_ip, int sport, int dport,
	int *sock, struct sockaddr_in *broker_addr)
{
	struct sockaddr_in src_addr;

        if ((*sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
                diep("Failed to obtain socket.");

        int optval = 1;
        setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        memset((char *) broker_addr, 0, sizeof(struct sockaddr_in));
        broker_addr->sin_family = AF_INET;
        broker_addr->sin_port = htons(dport);
        broker_addr->sin_addr.s_addr = inet_addr(broker_ip);

        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.sin_family = AF_INET;
        src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        src_addr.sin_port = htons(sport);

        if (bind(*sock, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0)
                diep("Failed to bind to source port.");
}

int main(int argc, char **argv)
{
	struct arguments arguments;

	// Default values
	arguments.broker_port = 42425;
	arguments.local_port  = 42425;
	arguments.interval    = 10;
	arguments.verbose     = 0;

	// Parse arguments
	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	struct sockaddr_in broker_addr;
	int socket;
	open_socket(
		arguments.broker_ip,
		arguments.local_port, arguments.broker_port,
		&socket, &broker_addr);

	printf(	"Sending UDP heartbeats to %s:%d from source port %d...\n",
		arguments.broker_ip, arguments.broker_port, arguments.local_port);

	while (1)
	{
		if (sendto(socket, arguments.secret, strlen(arguments.secret), 0,
		    (struct sockaddr*) &broker_addr, sizeof(broker_addr)) < 0)
		{
			diep("Failed to send heartbeat");
		}
		if (arguments.verbose)
		{
			printf(".");
			fflush(stdout);
		}

		sleep(arguments.interval);
	}

	close(socket);
	return 0;
}
