#ifndef __TSP_H
#define __TSP_H

#include "job.h"

typedef struct {
	int n_towns;
	int to_city[MAX_TOWNS][MAX_TOWNS];
	int dist [MAX_TOWNS][MAX_TOWNS];
} distance_matrix_t;

void init_tsp(distance_matrix_t *distance_matrix, job_queue_t *q);
void tsp (int hops, int len, path_t *path, int *cuts, int num_worker);
void distributor (int hops, int len, path_t *path);
void generate_jobs (void);
void *worker (void *num_worker);

#endif