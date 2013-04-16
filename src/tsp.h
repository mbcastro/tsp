#ifndef __TSP_H
#define __TSP_H

#include "job.h"

/*
 * We pad the distance structure to avoid false sharing 
 * during the execution. We combine this passing with the 
 * __attribute__ ((aligned (PAGE_SIZE))) directive on the 
 * variable declaration
 */
#define DISTANCE_MATRIX_T_SIZE (sizeof(int) * (1 + 2 * MAX_TOWNS * MAX_TOWNS))
#define DISTANCE_MAATRIX_T_PADDING PAGE_SIZE - DISTANCE_MATRIX_T_SIZE + (DISTANCE_MATRIX_T_SIZE / PAGE_SIZE * PAGE_SIZE)
typedef struct {
	int n_towns;
	struct {
		int to_city;
		int dist;
	} info[MAX_TOWNS][MAX_TOWNS];
	char PADDING[DISTANCE_MAATRIX_T_PADDING];
} distance_matrix_t;


void init_tsp(distance_matrix_t *distance_matrix, job_queue_t *q, int partition, int n_partitions, int n_workers, int n_towns);
void tsp (int hops, int len, path_t *path, unsigned long *cuts, int num_worker);
void generate_jobs ();
void *worker (void *num_worker);
int tsp_get_shortest_path (void);
void tsp_log_shortest_path (void);

#endif
