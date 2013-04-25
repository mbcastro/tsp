#include "job.h"

#ifdef NO_CACHE_COHERENCE //See close_queue() for details
static int waiting_threads = 0;
#endif


void init_queue (job_queue_t *q, unsigned long max_size) {
	q->max_size = max_size;
	q->begin = 0;
	q->end = 0;
	q->closed = 0;
	
	LOG("Trying to allocate %lu bytes for the queue\n", sizeof(job_queue_node_t) * max_size);
	q->buffer = (job_queue_node_t *) malloc(sizeof(job_queue_node_t) * max_size);
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

#ifdef NO_CACHE_COHERENCE
	__k1_rmb();
#endif

	if(q->begin == q->end && q->closed)
		return QUEUE_CLOSED;
	
	COND_VAR_MUTEX_LOCK(q->cond_var);
	while (q->begin == q->end) {
		if (q->closed)	{
			COND_VAR_MUTEX_UNLOCK(q->cond_var);
			return QUEUE_CLOSED;
		}
#ifdef NO_CACHE_COHERENCE
		waiting_threads++; //see close_queue()
		COND_VAR_WAIT(q->cond_var);
		waiting_threads--;
#else
		COND_VAR_WAIT(q->cond_var);
#endif		
	}

	index = q->begin++;
	COND_VAR_MUTEX_UNLOCK(q->cond_var);
	memcpy(j, &q->buffer[index].tsp_job, sizeof(job_t));		
	return QUEUE_OK;
} 

void close_queue (job_queue_t *q) {
	COND_VAR_MUTEX_LOCK(q->cond_var);
	q->closed = 1;
#ifdef NO_CACHE_COHERENCE
	int i;
	for (i = 0; i < waiting_threads; i++) //Dirty trick to solve the problem of pthread_broadcast on MPPA
		COND_VAR_SIGNAL(q->cond_var);
#else
	COND_VAR_BROADCAST(q->cond_var);
#endif
	COND_VAR_MUTEX_UNLOCK(q->cond_var);
}

void free_queue (job_queue_t *q) {
	free(q->buffer);
}
