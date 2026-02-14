#include "bakery.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static int ticket_counter = 0;
static int current_serving = 1;
static pthread_mutex_t sync_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct Ticket thread_result;

void *get_ticket_worker(void *arg)
{

	int ticket_number = ++ticket_counter;

	thread_result.number = ticket_number;

	pthread_mutex_unlock(&sync_mutex);

	printf("Issued ticket #%d\n", ticket_number);

	return NULL;
}

bool_t
get_ticket_1_svc(void *argp, struct Ticket *result, struct svc_req *rqstp)
{
	pthread_t thread;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_mutex_lock(&sync_mutex);

	pthread_create(&thread, &attr, get_ticket_worker, NULL);
	pthread_attr_destroy(&attr);

	pthread_mutex_lock(&sync_mutex);

	result->number = thread_result.number;

	pthread_mutex_unlock(&sync_mutex);

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