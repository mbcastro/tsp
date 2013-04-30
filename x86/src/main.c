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
static int next_partition;
MUTEX_CREATE(main_lock, static);

//Initialization synchronization
COND_VAR_CREATE(sync_barrier, static);
static int running_count;


struct execution_parameters {
	int cluster;
	int nb_clusters;
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
	tsps[p->cluster] = init_execution(p->cluster, p->nb_clusters, p->nb_partitions, p->nb_threads, p->nb_towns, p->seed);
	
	wait_barrier (p->nb_clusters);
	start_execution(tsps[p->cluster]);
	wait_barrier (p->nb_clusters);
	end_execution(tsps[p->cluster]);

	free(params);
	return NULL;
}

pthread_t *spawn (int cluster, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed, char* machine) {
	pthread_t *tid = (pthread_t *)malloc (sizeof(pthread_t));
	struct execution_parameters *params = (struct execution_parameters*) malloc (sizeof(struct execution_parameters));
	params->cluster = cluster;
	params->nb_clusters = nb_clusters;
	params->nb_partitions = nb_partitions;
	params->nb_threads = nb_threads;
	params->nb_towns = nb_towns;
	params->seed = seed;

	int status = pthread_create (tid, NULL, spawn_worker, params);
	assert (status == 0);

	if (machine) {
		char **machine_sched = get_machine_sched(machine);
		cpu_set_t *cpu_set = mask_for_partition(cluster, machine_sched);
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
		tids[i] = spawn(i, nb_clusters, nb_partitions, nb_threads, nb_towns, seed, machine);
	for (i = 0; i < nb_clusters; i++) {
		pthread_join (*(tids[i]), NULL);
		free(tids[i]);
	}

	free (tids);
	free(tsps);

    unsigned long exec_time = diff_time(start, get_time());
	printf ("%lu\t%d\t%d\t%d\t%d\t%d\t%d", 
		exec_time, min_distance,nb_threads, nb_towns, seed, nb_clusters, nb_partitions);

}

int main (int argc, char **argv) {

	CHECK_PAGE_SIZE();

	if (argc != 5 && argc != 6 && argc != 7) {
		fprintf (stderr, "Usage: %s <nb_threads> <nb_towns> <seed> <nb_clusters> [nb_executions machine]\n", argv[0]);
		return 1;
	}

	init_time();
	COND_VAR_INIT(sync_barrier);
	MUTEX_INIT(main_lock);
	
	int *nb_threads = par_parse (argv[1]);
	assert(nb_threads);
	int *nb_towns = par_parse (argv[2]);
	assert(nb_towns);
	int seed = atoi(argv[3]);
	int *nb_clusters = par_parse (argv[4]);
	assert(nb_clusters);
	int nb_executions = (argc == 6) ? atoi(argv[5]) : 1;
	assert(nb_executions > 0);
	char *machine = (argc == 7) ? argv[6] : NULL;
	
	int execution, town, cluster, thread;
	town = 0;
	while (nb_towns[town] != 0) {
		cluster = 0;
		while (nb_clusters[cluster] != 0) {
			thread = 0;
			while (nb_threads[thread] != 0) {
				assert (nb_threads[thread] > 0);
				assert (nb_towns[town] <= MAX_TOWNS);
				assert (nb_clusters[cluster] > 0);
				for (execution = 0; execution < nb_executions; execution++)
					run_tsp(nb_threads[thread], nb_towns[town], seed, nb_clusters[cluster], machine);
				thread++;
			}
			cluster++;
		}
		town++;
	}
	
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

static inline partition_interval_t get_next_partition_block_size(tsp_t_pointer tsp, int block_size) {
	partition_interval_t ret;
	ret.start = ret.end = -1;	
	if (next_partition < tsp->nb_partitions)
		ret.start = next_partition;
		if (next_partition + block_size - 1 < tsp->nb_partitions)
			ret.end = next_partition + block_size - 1;
		else
			ret.end = ret.start + tsp->nb_partitions - ret.start;
		next_partition += block_size;
	return ret;
}

inline partition_interval_t get_next_partition_block(tsp_t_pointer tsp) {	
	MUTEX_LOCK(main_lock);
	partition_interval_t ret = get_next_partition_block_size(tsp, 5);
	MUTEX_UNLOCK(main_lock);
	return ret;
}

inline partition_interval_t get_next_partition_fss(tsp_t_pointer tsp, int alfa) {
	MUTEX_LOCK(main_lock);
	int block_size;
	if (tsp->processed_partitions == 0)
		block_size = (tsp->nb_partitions / (1.0 / (INITIAL_JOB_DISTRIBUTION_PERCENTAGE / 100.0)) / tsp->nb_clusters);
	else
		block_size = (tsp->nb_partitions - next_partition) / (tsp->nb_clusters * alfa);
	if (block_size < 1) 
		block_size = 1;
	partition_interval_t ret = get_next_partition_block_size(tsp, block_size);
	MUTEX_UNLOCK(main_lock);
	return ret;
}

inline partition_interval_t get_next_partition_gss(tsp_t_pointer tsp) {
	return get_next_partition_fss(tsp, 1);
}

partition_interval_t get_next_partition(tsp_t_pointer tsp) {	
	return get_next_partition_gss(tsp);
}