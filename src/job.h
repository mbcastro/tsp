#ifndef __JOB_H
#define __JOB_H

#include "defs.h"

typedef int path_t[MAX_TOWNS];

typedef struct {
	int len;
	path_t path;
} job_t;

typedef struct job_queue_node {
	job_t tsp_job;
	struct job_queue_node *next;
} job_queue_node_t;

typedef struct {
	unsigned long max_size;
	int begin;
	int end;
	int closed;
	job_queue_node_t *buffer;
	MUTEX_CREATE(mutex);
} job_queue_t;

void init_queue (job_queue_t *q, unsigned long max_size);
void free_queue (job_queue_t *q);

void add_job (job_queue_t *q, job_t j);

typedef enum {
	QUEUE_CLOSED = 0,
	QUEUE_RETRY = 1,
	QUEUE_OK = 2
} queue_status_t;

queue_status_t get_job (job_queue_t *q, job_t *j);

void close_queue (job_queue_t *q);
int is_queue_closed (job_queue_t *q);

#endif
