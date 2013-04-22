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

typedef struct {
	distance_matrix_t *distance; //first element to force alignement
	job_queue_t queue;
	int max_hops;
	int min_distance;
	MUTEX_CREATE(mutex);
} tsp_t;

typedef struct {
	tsp_t *tsp;
	int num_worker;
} tsp_worker_par_t;

tsp_t *init_tsp(int partition, int n_partitions, int n_workers, int n_towns, int seed);
void free_tsp(tsp_t *tsp);
void tsp (tsp_t *tsp, int hops, int len, path_t *path, unsigned long *cuts, int num_worker);
void generate_jobs (tsp_t *tsp, const int partition, const int n_partitions);
void *worker (void *tsp_worker_par);
int tsp_get_shortest_path (tsp_t *tsp);
void tsp_log_shortest_path (tsp_t *tsp);
int tsp_update_minimum_distance(tsp_t *tsp, int new_distance);

//callback
extern void new_minimun_distance_found(tsp_t *tsp, int num_worker, int length);
//minimum hint
void update_minimum(tsp_t *tsp, int new_minimum);

#endif
