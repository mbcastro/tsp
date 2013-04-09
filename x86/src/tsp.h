#ifndef __TSP_H
#define __TSP_H

#include "job.h"

typedef struct {
	int n_towns;
	int to_city[MAX_TOWNS][MAX_TOWNS];
	int dist [MAX_TOWNS][MAX_TOWNS];
} distance_matrix_t;

void init_tsp(distance_matrix_t *distance_matrix, job_queue_t *q, int nb_workers, int n_towns);
void tsp (int hops, int len, path_t *path, unsigned long *cuts, int num_worker);
unsigned long distributor (int hops, int len, path_t *path);
void generate_jobs ();
void *worker (void *num_worker);
void tsp_log_shortest_path (void);

#endif