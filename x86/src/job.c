#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "job.h"

void init_queue (job_queue_t *q) {
	q->first = NULL;
	q->last = NULL;
	MUTEX_INIT(q->mutex);
}

void add_job (job_queue_t *q, job_t j) {
	job_queue_node_t *ptr;

	ptr = (job_queue_node_t *) malloc (sizeof (job_queue_node_t));
	ptr->next = NULL;
	ptr->tsp_job.len = j.len;

	memcpy (&ptr->tsp_job.path, j.path, sizeof(path_t));

	// The job generation is done sequentially
	// MUTEX_LOCK(q->mutex);

	if (q->first == NULL)
		q->first = q->last = ptr;
	else {
		q->last->next = ptr;
		q->last = ptr;
	}

	// MUTEX_UNLOCK(q->mutex);
}

int get_job (job_queue_t *q, job_t *j) {
	job_queue_node_t *ptr = NULL;

	MUTEX_LOCK(q->mutex);

	if (q->first == NULL) {
		MUTEX_UNLOCK(q->mutex);
		return 0;
	}

	ptr = q->first;
	q->first = ptr->next;

	if (q->first == NULL)
		q->last = NULL;

	MUTEX_UNLOCK(q->mutex);

	memcpy(j, &ptr->tsp_job, sizeof(job_t));

	free (ptr);
	return 1;
} 

