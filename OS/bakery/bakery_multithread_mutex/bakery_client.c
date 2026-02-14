#include "bakery.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

void bakery_prog_1(char *host)
{
	CLIENT *clnt;
	enum clnt_stat retval_1;
	struct Ticket ticket_result;
	enum clnt_stat retval_2;
	struct ServiceRequest service_arg;
	struct ServiceResponse service_result;
	pid_t client_pid = getpid();

	clnt = clnt_create(host, BAKERY_PROG, BAKERY_VER, "udp");
	if (clnt == NULL)
	{
		clnt_pcreateerror(host);
		exit(1);
	}

	for (int i = 0; i < 500; i++)
	{

		retval_1 = get_ticket_1(NULL, &ticket_result, clnt);
		if (retval_1 != RPC_SUCCESS)
		{
			clnt_perror(clnt, "get_ticket failed");
			continue;
		}

		int ticket_number = ticket_result.number;
		printf("Client #%d got ticket #%d\n", client_pid, ticket_number);

		usleep(rand() % 500000);

		service_arg.ticket_number = ticket_number;

		do
		{
			retval_2 = get_service_1(&service_arg, &service_result, clnt);
			if (retval_2 != RPC_SUCCESS)
			{
				clnt_perror(clnt, "get_service failed");
				break;
			}

			if (service_result.result == NOT_YOUR_QUEUE)
			{
				printf("Не моя очередь\n");
				usleep(100000);
			}
		} while (service_result.result == NOT_YOUR_QUEUE);

		if (service_result.result != NOT_YOUR_QUEUE)
		{
			printf("Client #%d got result: %ld\n", client_pid, service_result.result);
		}
		else
		{
			printf("Client #%d failed to get service after retries\n", client_pid);
		}
	}

	clnt_destroy(clnt);
}

int main(int argc, char *argv[])
{
	char *host;
	srand(time(NULL) + getpid());

	if (argc < 2)
	{
		printf("usage: %s server_host\n", argv[0]);
		exit(1);
	}

	host = argv[1];
	bakery_prog_1(host);
	exit(0);
}