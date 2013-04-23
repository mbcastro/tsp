#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "exec.h"
#include "timer.h"

static tsp_t_pointer *tsps;

static int min_distance = INT_MAX;
MUTEX_CREATE(min_lock, static);

//Initialization synchronization
COND_VAR_CREATE(sync_barrier, static);
static int running_count = 0;


struct execution_parameters {
	int partition;
	int nb_partitions;
	int nb_threads;
	int nb_towns;
	int seed;
};

void wait_barrier (int limit) {
	COND_VAR_MUTEX_LOCK(sync_barrier);
	running_count++;
	if (running_count == limit) {
		running_count = 0;
		COND_VAR_BROADCAST(sync_barrier);
	} else 
		COND_VAR_WAIT(sync_barrier);
	COND_VAR_MUTEX_UNLOCK(sync_barrier);
}
 
void *spawn_worker(void* params) {
	struct execution_parameters *p = (struct execution_parameters *)params;
	tsps[p->partition] = init_execution(p->partition, p->nb_partitions, p->nb_threads, p->nb_towns, p->seed);
	
	wait_barrier (p->nb_partitions);
	start_execution(tsps[p->partition]);
	wait_barrier (p->nb_partitions);
	end_execution(tsps[p->partition]);

	free(params);
	return NULL;
}

pthread_t *spawn (int partition, int nb_partitions, int nb_threads, int nb_towns, int seed) {
	pthread_t *tid = (pthread_t *)malloc (sizeof(pthread_t));
	struct execution_parameters *params = (struct execution_parameters*) malloc (sizeof(struct execution_parameters));
	params->partition = partition;
	params->nb_partitions = nb_partitions;
	params->nb_threads = nb_threads;
	params->nb_towns = nb_towns;
	params->seed = seed;

	pthread_create (tid, NULL, spawn_worker, params);
	return tid;	
}


int main (int argc, char **argv) {
	int i;

	CHECK_PAGE_SIZE();
	if (argc != 5) {
		fprintf (stderr, "Usage: %s <nb_threads> <nb_towns> <seed> <nb_partitions>\n", argv[0]);
		return 1;
	}

	unsigned long start = get_time();	

	COND_VAR_INIT(sync_barrier);
	MUTEX_INIT(min_lock);
	int nb_threads = atoi(argv[1]);
	int nb_towns = atoi(argv[2]);
	int seed = atoi(argv[3]);
	int nb_partitions = atoi(argv[4]);

	LOG ("nb_threads = %3d nb_towns = %3d seed = %d nb_partitions = %d\n", 
		nb_threads, nb_towns, seed, nb_partitions);

	tsps = 	(tsp_t_pointer *)(malloc(sizeof(tsp_t_pointer) * nb_partitions));
	assert(tsps != NULL);
	pthread_t **tids = (pthread_t **) malloc (sizeof(pthread_t *) * nb_partitions);
	assert (tids != NULL);	
	for (i = 0; i < nb_partitions; i++)
		tids[i] = spawn(i, nb_partitions, nb_threads, nb_towns, seed);
	for (i = 0; i < nb_partitions; i++) {
		pthread_join (*(tids[i]), NULL);
		free(tids[i]);
	}

	free (tids);
	free(tsps);

    unsigned long exec_time = diff_time(start, get_time());
	printf ("%lu\t%d\t%d\t%d\t%d\t%d\n", 
		exec_time, min_distance,nb_threads, nb_towns, seed, nb_partitions);

	return 0;
}

void new_minimun_distance_found(tsp_t_pointer tsp) {
	int i;
	for (i = 0; i < tsp->nb_partitions; i++) {
		if (tsps[i] != tsp)
			tsp_update_minimum_distance (tsps[i], tsp->min_distance);
	}
	if (min_distance > tsp->min_distance) {
		MUTEX_LOCK(min_lock);
		if (min_distance > tsp->min_distance)
			min_distance = tsp->min_distance;
		MUTEX_UNLOCK(min_lock);
	}
}

