#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "job.h"

void init_queue (job_queue_t *q, unsigned long max_size) {
	q->max_size = max_size;
	q->begin = 0;
	q->end = 0;
	q->buffer = (job_queue_node_t *) malloc(sizeof(job_queue_node_t) * max_size);
	LOG("Trying to allocate %lu bytes for the queue\n", sizeof(job_queue_node_t) * max_size);
	assert(q->buffer != NULL);
	MUTEX_INIT(q->mutex);
}

void add_job (job_queue_t *q, job_t j) {

	// The job generation is done sequentially
	// MUTEX_LOCK(q->mutex);
	q->buffer[q->end].tsp_job.len = j.len;
	memcpy (&q->buffer[q->end].tsp_job.path, j.path, sizeof(path_t));
	q->end++;
	// MUTEX_UNLOCK(q->mutex);
}

int get_job (job_queue_t *q, job_t *j) {
	int index;

	if(q->begin == q->end)
		return 0;

	MUTEX_LOCK(q->mutex);
	if(q->begin == q->end) {
		MUTEX_UNLOCK(q->mutex);
		return 0;
	}
	index = q->begin++;
	MUTEX_UNLOCK(q->mutex);
	
	memcpy(j, &q->buffer[index].tsp_job, sizeof(job_t));	
	
	return 1;
} 

void free_queue (job_queue_t *q) {
	free(q->buffer);
}
