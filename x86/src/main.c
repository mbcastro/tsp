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
/*	
	pthread_attr_t att;	
	pthread_attr_init (&att);
    pthread_attr_setstacksize (&att, 20000); 
*/
	tids = (pthread_t *) malloc (sizeof(pthread_t) * nb_workers);

	for (i = 0; i < nb_workers - 1; i++) {
		pthread_create (&tids[i], NULL, worker, (void *)i);
	}

	worker ((void *)((long)nb_workers - 1));

	for (i = 0; i < nb_workers - 1; i++) {
		pthread_join (tids[i], &status);
	}
#else
	worker ((void *)1);
#endif

}

int main (int argc, char **argv) {
	distance_matrix_t distance __attribute__ ((aligned (PAGE_SIZE)));
	int n_workers, n_towns, seed;
	job_queue_t q;
	unsigned long start, end, end_generation, diff, diff_generation;

	if (argc != 4) {
		fprintf (stderr, "Usage: %s <nb_threads > <ncities> <seed> \n",argv[0]);
		exit (1);
	}

	n_workers = atoi (argv[1]);
	n_towns = atoi(argv[2]);
	seed = atoi(argv[3]);

	LOG ("nb_threads = %3d ncities = %3d\n", n_workers, n_towns);

	init_time();
	init_queue (&q);
	init_distance (&distance, n_towns, seed);
	init_tsp(&distance, &q, n_workers, n_towns);
	start = get_time();
	generate_jobs ();
	end_generation = get_time();
	do_work(n_workers);
	end = get_time();

	diff = diff_time (start, end);
	diff_generation = diff_time(start, end_generation);
	tsp_log_shortest_path();
	printf("time = %lu generation = %lu (Ratio %.4f)\n", diff, diff_generation, 100.0f * diff_generation / diff);
	
	return 0;
}


