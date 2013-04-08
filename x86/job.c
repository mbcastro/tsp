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
	unsigned int i;

	ptr = (job_queue_node_t *) malloc (sizeof (job_queue_node_t));
	ptr->next = NULL;
	ptr->tsp_job.len=j.len;
	for (i = 0; i < MAX_TOWNS; i++)
		ptr->tsp_job.path [i] = j.path [i];

	MUTEX_LOCK(q->mutex);

	if (q->first == NULL)
		q->first = q->last = ptr;
	else {
		q->last->next = ptr;
		q->last = ptr;
	}

	MUTEX_UNLOCK(q->mutex);
}


int get_job (job_queue_t *q, job_t *j) {
	job_queue_node_t *ptr;

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

