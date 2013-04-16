#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include "exec.h"
#include "timer.h"
#include "defs.h"
#include "tsp.h"
#include "job.h"

void genmap (distance_matrix_t* distance_matrix, int seed) {
	int n = distance_matrix->n_towns;	
	int x[n], y[n];
	int tempdist [n];	
	int i, j, k, city = 0;
	int dx, dy, tmp;

	RAND_INIT(seed);

	for (i = 0; i < n; i++) {
		x[i] = RAND_NEXT() % MAX_GRID_X;
		y[i] = RAND_NEXT() % MAX_GRID_Y;
	}

	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			dx = x[i] - x[j];
			dy = y[i] - y[j];
			tempdist [j] = (int) sqrt ((double) ((dx * dx) + (dy * dy)));
		}

		for (j = 0; j < n; j++) {
			tmp = INT_MAX;
			for (k = 0; k < n; k++) {
				if (tempdist [k] < tmp) {
					tmp = tempdist [k];
					city = k;
				}
			}
			tempdist [city] = INT_MAX;
			distance_matrix->info[i][j].to_city = city;
			distance_matrix->info[i][j].dist = tmp;
		}
	}
}


void init_distance (distance_matrix_t *distance, int n_towns, int seed) {
	distance->n_towns = n_towns;
	genmap (distance, seed); 
}

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



 execution_info_t do_work (const int partition, const int n_partitions, const int nb_workers) {
	execution_info_t ret;
#ifdef MT
	unsigned long i;
	void *status;
	pthread_t *tids;

	tids = (pthread_t *) malloc (sizeof(pthread_t) * (nb_workers - 1));
	assert (tids != NULL);

	for (i = 0; i < nb_workers - 1; i++)
		pthread_create (&tids[i], NULL, worker, (void *)i);
	
	generate_jobs(partition, n_partitions);

	worker ((void *)((long)nb_workers - 1));

	for (i = 0; i < nb_workers - 1; i++)
		pthread_join (tids[i], &status);
	free (tids);
#else
	generate_jobs(partition, n_partitions);

	worker ((void *)1);
#endif
	ret.partition = partition;
	ret.shortest_path_length = tsp_get_shortest_path();
	return ret;
}

execution_info_t start_execution(int partition, int n_partitions, int n_workers, int n_towns, int seed) {
	distance_matrix_t *distance;
	job_queue_t *q;
	execution_info_t ret;

	distance = (distance_matrix_t *) malloc(sizeof(distance_matrix_t));
	q = (job_queue_t *) malloc(sizeof(job_queue_t));
	assert(distance != NULL && q != NULL);

	LOG ("nb_threads = %3d ncities = %3d seed = %d\n", n_workers, n_towns, seed);

	init_time();
	init_distance (distance, n_towns, seed);
	init_tsp(distance, q, partition, n_partitions, n_workers, n_towns);

	ret = do_work(partition, n_partitions, n_workers);

	free_queue(q);
	free(distance);
	free(q);

	return ret;
}


void print_execution_info(execution_info_t exec_info) {
	printf("Partition: %d Shortest path length: %d\n", exec_info.partition, exec_info.shortest_path_length);
}