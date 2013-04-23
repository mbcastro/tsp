#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "exec.h"
#include "timer.h"
#include "defs.h"
#include "tsp.h"
#include "job.h"


void print_distance_matrix (distance_matrix_t *distance) {
	int i, j;

	LOG ("distance.n_towns = %d\n", distance->n_towns);

	for (i = 0; i<distance->n_towns; i++) {
		LOG ("distance.dst [%1d]",i);
		for (j = 0; j<distance->n_towns; j++) {
			LOG (" [d:%2d, to:%2d] ", distance->info[i][j].dist, distance->info[i][j].to_city);
		}
		LOG (";\n\n");
	}
	LOG ("done ...\n");
}



 execution_info_t do_work (tsp_t *tsp) {
	execution_info_t ret;
#ifdef MT
	unsigned long i;
	pthread_t *tids = NULL;

	if(tsp->nb_workers > 1) {
		tids = (pthread_t *) malloc (sizeof(pthread_t) * (tsp->nb_workers - 1));
		assert (tids != NULL);
	}

	for (i = 0; i < tsp->nb_workers - 1; i++) {
		int status;
		tsp_worker_par_t *par = (tsp_worker_par_t *) malloc(sizeof (tsp_worker_par_t));
		par->tsp = tsp;
		par->num_worker = i;
		status = pthread_create (&tids[i], NULL, worker, par);
		assert (status == 0);
	}

	generate_jobs(tsp);

	tsp_worker_par_t *par = (tsp_worker_par_t *) malloc(sizeof (tsp_worker_par_t));
	par->tsp = tsp;
	par->num_worker = tsp->nb_workers - 1;
	worker (par);

	for (i = 0; i < tsp->nb_workers - 1; i++)
		pthread_join (tids[i], NULL);
	free (tids);
#else
	generate_jobs(tsp);
	tsp_worker_par_t *par = (tsp_worker_par_t *) malloc(sizeof (tsp_worker_par_t));
	assert (par != NULL);
	par->tsp = tsp;
	par->num_worker = 1;
	worker (par);
#endif

	ret.partition = tsp->partition;
	ret.shortest_path_length = tsp_get_shortest_path(tsp);
	return ret;
}

tsp_t_pointer init_execution(int partition, int n_partitions, int n_workers, int n_towns, int seed) {
	init_time();
	tsp_t_pointer ret = init_tsp(partition, n_partitions, n_workers, n_towns, seed);
	LOG("Created TSP %d %p\n", partition, ret);
	return ret;
}

execution_info_t start_execution(tsp_t_pointer tsp) {
	execution_info_t ret;
	ret = do_work(tsp);
	return ret;
}

void end_execution (tsp_t_pointer tsp) {
	free_tsp(tsp);
}

void print_execution_info(execution_info_t exec_info) {
	printf("Partition: %d Shortest path length: %d\n", exec_info.partition, exec_info.shortest_path_length);
}