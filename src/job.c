#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "job.h"

void init_queue (job_queue_t *q, unsigned long max_size) {
	q->max_size = max_size;
	q->begin = 0;
	q->end = 0;
	q->closed = 0;
	q->buffer = (job_queue_node_t *) malloc(sizeof(job_queue_node_t) * max_size);
	LOG("Trying to allocate %lu bytes for the queue\n", sizeof(job_queue_node_t) * max_size);
	assert(q->buffer != NULL);
	COND_VAR_INIT(q->cond_var);
}

void add_job (job_queue_t *q, job_t j) {
	COND_VAR_MUTEX_LOCK(q->cond_var);
	q->buffer[q->end].tsp_job.len = j.len;
	memcpy (&q->buffer[q->end].tsp_job.path, j.path, sizeof(path_t));
	q->end++;
	COND_VAR_SIGNAL(q->cond_var);
	COND_VAR_MUTEX_UNLOCK(q->cond_var);
}

queue_status_t get_job (job_queue_t *q, job_t *j) {
	int index;

#ifndef NO_CACHE_COHERENCE
	if(q->begin == q->end && is_queue_closed(q))
		return QUEUE_CLOSED;
#endif
	COND_VAR_MUTEX_LOCK(q->cond_var);
	while (q->begin == q->end) {
		if (is_queue_closed(q))	{
			COND_VAR_MUTEX_UNLOCK(q->cond_var);
			return QUEUE_CLOSED;
		}
		COND_VAR_WAIT(q->cond_var);
	}

	index = q->begin++;
	COND_VAR_MUTEX_UNLOCK(q->cond_var);
	memcpy(j, &q->buffer[index].tsp_job, sizeof(job_t));		
	return QUEUE_OK;
} 

void close_queue (job_queue_t *q) {
	q->closed = 1;
}

inline int is_queue_closed (job_queue_t *q) {
	return q->closed;
}

void free_queue (job_queue_t *q) {
	free(q->buffer);
}
