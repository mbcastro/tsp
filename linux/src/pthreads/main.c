#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sched.h>
#include <errno.h>

#include "cpu_affinity.h"
#include "common_main.h"

static tsp_t_pointer *tsps;

static int min_distance;
static int next_partition;
MUTEX_CREATE(main_lock, static);

//Initialization synchronization
COND_VAR_CREATE(sync_barrier, static);
static int running_count;


int main (int argc, char **argv) {

	struct main_pars pars = init_main_pars(argc, argv);
	
	COND_VAR_INIT(sync_barrier);
	MUTEX_INIT(main_lock);	
	
	run_main(pars);	
	
	free_main(pars);

	return 0;
}

void new_minimun_distance_found(tsp_t_pointer tsp) {
	int i;
	int min = tsp_get_shortest_path(tsp);
	for (i = 0; i < tsp->nb_clusters; i++) {
		if (tsps[i] != tsp)
			tsp_update_minimum_distance (tsps[i], min);
	}
	if (min_distance > min) {
		MUTEX_LOCK(main_lock);
		if (min_distance > min)
			min_distance = min;
		MUTEX_UNLOCK(main_lock);
	}
}

void wait_barrier (barrier_par_t barrier_par) {
	int limit = barrier_par.int_t;
	COND_VAR_MUTEX_LOCK(sync_barrier);
	running_count++;
	if (running_count == limit) {
		running_count = 0;
		COND_VAR_BROADCAST(sync_barrier);
	} else 
		COND_VAR_WAIT(sync_barrier);
	COND_VAR_MUTEX_UNLOCK(sync_barrier);
} 


pthread_t *spawn (tsp_t_pointer *tsp, int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed, char* machine) {
	pthread_t *tid = (pthread_t *)malloc (sizeof(pthread_t));
	struct execution_parameters *params = (struct execution_parameters*) malloc (sizeof(struct execution_parameters));
	params->cluster = cluster_id;
	params->nb_clusters = nb_clusters;
	params->nb_partitions = nb_partitions;
	params->nb_threads = nb_threads;
	params->nb_towns = nb_towns;
	params->seed = seed;
	params->tsp = tsp;
	params->barrier.int_t = nb_clusters;

	int status = pthread_create (tid, NULL, spawn_worker, params);
	assert (status == 0);

	if (machine) {
		char **machine_sched = get_machine_sched(machine);
		cpu_set_t *cpu_set = mask_for_partition(cluster_id, machine_sched);
		status = pthread_setaffinity_np (*tid, sizeof(cpu_set_t), cpu_set);
		assert (status == 0);
		free(cpu_set);
	}

	return tid;	
}

void run_tsp (int nb_threads, int nb_towns, int seed, int nb_clusters, char* machine) {
	int i;

	unsigned long start = get_time();
	int nb_partitions = nb_clusters * PARTITIONS_PER_CLUSTER;
	LOG ("nb_clusters = %3d nb_partitions = %3d nb_threads = %3d nb_towns = %3d seed = %d \n", 
		nb_clusters, nb_partitions, nb_threads, nb_towns, seed);

	min_distance = INT_MAX;
	next_partition = 0;
	running_count = 0;
	tsps = 	(tsp_t_pointer *)(malloc(sizeof(tsp_t_pointer) * nb_clusters));
	assert(tsps != NULL);
	pthread_t **tids = (pthread_t **) malloc (sizeof(pthread_t *) * nb_clusters);
	assert (tids != NULL);	
	for (i = 0; i < nb_clusters; i++)
		tids[i] = spawn(&tsps[i], i, nb_clusters, nb_partitions, nb_threads, nb_towns, seed, machine);
	for (i = 0; i < nb_clusters; i++) {
		pthread_join (*(tids[i]), NULL);
		free(tids[i]);
	}

	free (tids);
	free(tsps);

    unsigned long exec_time = diff_time(start, get_time());
	printf ("%15lu\t%5d\t%2d\t%2d\t%5d\t%2d\t%8d", 
		exec_time, min_distance,nb_threads, nb_towns, seed, nb_clusters, nb_partitions);
}

partition_interval_t get_next_partition(tsp_t_pointer tsp) {
	partition_interval_t ret;
	MUTEX_LOCK(main_lock);
	ret = get_next_partition_default_impl(tsp->nb_partitions, tsp->nb_clusters, &next_partition, tsp->processed_partitions);
	MUTEX_UNLOCK(main_lock);
	return ret;
}
