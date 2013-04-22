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
	MUTEX_INIT(q->mutex);
	COND_VAR_INIT(q->cond);
}

void add_job (job_queue_t *q, job_t j) {
	MUTEX_LOCK(q->mutex);
	q->buffer[q->end].tsp_job.len = j.len;
	memcpy (&q->buffer[q->end].tsp_job.path, j.path, sizeof(path_t));
	q->end++;
	COND_VAR_SIGNAL(q->cond);
	MUTEX_UNLOCK(q->mutex);
}

queue_status_t get_job (job_queue_t *q, job_t *j) {
	int index;

#ifndef NO_CACHE_COHERENCE
	if(q->begin == q->end && is_queue_closed(q))
		return QUEUE_CLOSED;
#endif
	MUTEX_LOCK(q->mutex);
	while (q->begin == q->end) {
		if (is_queue_closed(q))	{
			MUTEX_UNLOCK(q->mutex);
			return QUEUE_CLOSED;
		}
		COND_VAR_WAIT(q->cond, q->mutex);
	}

	index = q->begin++;
	MUTEX_UNLOCK(q->mutex);
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
