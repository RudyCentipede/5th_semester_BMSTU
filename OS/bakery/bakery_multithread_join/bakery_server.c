#include "bakery.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static int ticket_counter = 0;
static int current_serving = 1;

struct TicketThreadArgs
{
	struct Ticket result;
};

void *get_ticket_worker(void *arg)
{
	struct TicketThreadArgs *args = (struct TicketThreadArgs *)arg;

	args->result.number = ++ticket_counter;

	return NULL;
}

bool_t
get_ticket_1_svc(void *argp, struct Ticket *result, struct svc_req *rqstp)
{
	pthread_t thread;
	struct TicketThreadArgs args;

	pthread_create(&thread, NULL, get_ticket_worker, &args);

	pthread_join(thread, NULL);

	result->number = args.result.number;

	// printf("Issued ticket #%d\n", result->number);

	return TRUE;
}

bool_t
get_service_1_svc(struct ServiceRequest *argp, struct ServiceResponse *result, struct svc_req *rqstp)
{
	int ticket = argp->ticket_number;

	if (current_serving != ticket)
	{
		result->result = NOT_YOUR_QUEUE;
		return TRUE;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);
	long milliseconds = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	result->result = milliseconds > 0 ? milliseconds : labs(milliseconds);

	printf("Serving ticket #%d, result: %ld\n", ticket, result->result);

	current_serving++;

	return TRUE;
}

int bakery_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free(xdr_result, result);
	return 1;
}