#ifndef __JOB_H
#define __JOB_H

#include "defs.h"

typedef int path_t [MAX_TOWNS];

typedef struct {
	int len;
	path_t path;
} job_t;


typedef struct job_queue_node {
	job_t tsp_job;
	struct job_queue_node *next;
} job_queue_node_t;

typedef struct {
	job_queue_node_t *first;
	job_queue_node_t *last;
	MUTEX_CREATE(mutex);
} job_queue_t;

extern void init_queue (job_queue_t *q);
extern void add_job (job_queue_t *q, job_t j);
extern int get_job (job_queue_t *q, job_t *j);

#endif