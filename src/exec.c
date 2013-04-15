#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

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

void do_work (int nb_workers) {
#ifdef MT
	unsigned long i;
	void *status;
	pthread_t *tids;

	tids = (pthread_t *) malloc (sizeof(pthread_t) * (nb_workers - 1));
	assert (tids != NULL);

	for (i = 0; i < nb_workers - 1; i++)
		pthread_create (&tids[i], NULL, worker, (void *)i);

	worker ((void *)((long)nb_workers - 1));

	for (i = 0; i < nb_workers - 1; i++)
		pthread_join (tids[i], &status);
	free (tids);
#else
	worker ((void *)1);
#endif

}

int start_execution(int n_workers, int n_towns, int seed) {
	distance_matrix_t *distance;
	job_queue_t *q;
	unsigned long start, end, end_generation, diff, diff_generation;

	distance = (distance_matrix_t *) malloc(sizeof(distance_matrix_t));
	q = (job_queue_t *) malloc(sizeof(job_queue_t));
	assert(distance != NULL && q != NULL);

	LOG ("nb_threads = %3d ncities = %3d seed = %d\n", n_workers, n_towns, seed);

	init_time();
	init_distance (distance, n_towns, seed);
	init_tsp(distance, q, n_workers, n_towns);
	
	start = get_time();
	generate_jobs();
	end_generation = get_time();
	do_work(n_workers);
	end = get_time();

	free_queue(q);
	free(distance);
	free(q);

	diff = diff_time (start, end);
	diff_generation = diff_time(start, end_generation);
	tsp_log_shortest_path();
	printf("time = %lu generation = %lu (Ratio %.4f)\n", diff, diff_generation, 100.0f * diff_generation / diff);
	
	return tsp_get_shortest_path();
}
