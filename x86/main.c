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
	srand (seed);

	for (i = 0; i < n; i++) {
		x[i] = rand () % MAX_GRID_X;
		y[i] = rand () % MAX_GRID_Y;
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
			distance_matrix->to_city[i][j] = city;
			distance_matrix->dist[i][j] = tmp;
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
			LOG (" [d:%2d, to:%2d] ", distance->dist[i][j], distance->to_city[i][j]);
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
		LOG ("tid %x\n", (unsigned int)tids [i]);
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

	int nb_workers;
	job_queue_t q;
	distance_matrix_t distance;

	unsigned long start, end, diff;

	if (argc != 4) {
		fprintf (stderr, "Usage: %s <nb_threads > <ncities> <seed> \n",argv[0]);
		exit (1);
	}

	nb_workers = atoi (argv[1]);
	LOG ("nb_threads = %3d ncities = %3d\n", nb_workers, atoi(argv[2]));

	init_time();
	init_queue (&q);
	init_distance (&distance, atoi(argv[2]), atoi(argv[3]));
	init_tsp(&distance, &q);
	generate_jobs ();
	start = get_time();
	do_work(nb_workers);
	end = get_time();

	diff = diff_time (start, end);	
	printf("time = %lu\n", diff);
	return 0;
}


