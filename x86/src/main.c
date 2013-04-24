#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sched.h>
#include <errno.h>

#include "cpu_affinity.h"
#include "exec.h"
#include "timer.h"

static tsp_t_pointer *tsps;

static int min_distance;
MUTEX_CREATE(min_lock, static);

//Initialization synchronization
COND_VAR_CREATE(sync_barrier, static);
static int running_count;


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

pthread_t *spawn (int partition, int nb_partitions, int nb_threads, int nb_towns, int seed, char* machine) {
	pthread_t *tid = (pthread_t *)malloc (sizeof(pthread_t));
	struct execution_parameters *params = (struct execution_parameters*) malloc (sizeof(struct execution_parameters));
	params->partition = partition;
	params->nb_partitions = nb_partitions;
	params->nb_threads = nb_threads;
	params->nb_towns = nb_towns;
	params->seed = seed;

	int status = pthread_create (tid, NULL, spawn_worker, params);
	assert (status == 0);

	if (machine) {
		char **machine_sched = get_machine_sched(machine);
		cpu_set_t *cpu_set = mask_for_partition(partition, machine_sched);
		status = pthread_setaffinity_np (*tid, sizeof(cpu_set_t), cpu_set);
		assert (status == 0);
		free(cpu_set);
	}

	return tid;	
}


void run_tsp (int nb_threads, int nb_towns, int seed, int nb_partitions, char* machine) {
	int i;

	unsigned long start = get_time();
	LOG ("nb_threads = %3d nb_towns = %3d seed = %d nb_partitions = %d\n", 
		nb_threads, nb_towns, seed, nb_partitions);

	min_distance = INT_MAX;
	running_count = 0;
	tsps = 	(tsp_t_pointer *)(malloc(sizeof(tsp_t_pointer) * nb_partitions));
	assert(tsps != NULL);
	pthread_t **tids = (pthread_t **) malloc (sizeof(pthread_t *) * nb_partitions);
	assert (tids != NULL);	
	for (i = 0; i < nb_partitions; i++)
		tids[i] = spawn(i, nb_partitions, nb_threads, nb_towns, seed, machine);
	for (i = 0; i < nb_partitions; i++) {
		pthread_join (*(tids[i]), NULL);
		free(tids[i]);
	}

	free (tids);
	free(tsps);

    unsigned long exec_time = diff_time(start, get_time());
	printf ("%lu\t%d\t%d\t%d\t%d\t%d\n", 
		exec_time, min_distance,nb_threads, nb_towns, seed, nb_partitions);

}

int main (int argc, char **argv) {

	CHECK_PAGE_SIZE();

	if (argc != 5 && argc != 6 && argc != 7) {
		fprintf (stderr, "Usage: %s <nb_threads> <nb_towns> <seed> <nb_partitions> [nb_executions machine]\n", argv[0]);
		return 1;
	}

	init_time();
	COND_VAR_INIT(sync_barrier);
	MUTEX_INIT(min_lock);
	
	int *nb_threads = par_parse (argv[1]);
	assert(nb_threads);
	int *nb_towns = par_parse (argv[2]);
	assert(nb_towns);
	int seed = atoi(argv[3]);
	int *nb_partitions = par_parse (argv[4]);
	assert(nb_partitions);
	int nb_executions = (argc == 6) ? atoi(argv[5]) : 1;
	assert(nb_executions > 0);
	char *machine = (argc == 7) ? argv[6] : NULL;
	
	int execution, town, partition, thread;
	town = 0;
	while (nb_towns[town] != 0) {
		partition = 0;
		while (nb_partitions[partition] != 0) {
			thread = 0;
			while (nb_threads[thread] != 0) {
				assert (nb_threads[thread] > 0);
				assert (nb_towns[town] <= MAX_TOWNS);
				assert (nb_partitions[partition] > 0);
				for (execution = 0; execution < nb_executions; execution++)
					run_tsp(nb_threads[thread], nb_towns[town], seed, nb_partitions[partition], machine);
				thread++;
			}
			partition++;
		}
		town++;
	}
	
	return 0;
}

void new_minimun_distance_found(tsp_t_pointer tsp) {
	int i;
	int min = tsp_get_shortest_path(tsp);
	for (i = 0; i < tsp->nb_partitions; i++) {
		if (tsps[i] != tsp)
			tsp_update_minimum_distance (tsps[i], min);
	}
	if (min_distance > min) {
		MUTEX_LOCK(min_lock);
		if (min_distance > min)
			min_distance = min;
		MUTEX_UNLOCK(min_lock);
	}
}

