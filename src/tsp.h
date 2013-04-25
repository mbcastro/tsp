#ifndef __TSP_H
#define __TSP_H

#include "job.h"

typedef struct {
	int n_towns;
	struct {
		int to_city;
		int dist;
	} info[MAX_TOWNS][MAX_TOWNS];
} distance_matrix_t;

typedef struct {
	int max_hops;
	int partition;
	int nb_partitions;
	int nb_workers;
	char PADDING1[PADDING(4 * sizeof (int))];

	distance_matrix_t *distance;
	char PADDING2[PADDING(sizeof(distance_matrix_t))];

	int min_distance;
	MUTEX_CREATE(mutex);
	char PADDING4[PADDING(sizeof(int) + MUTEX_SIZE)];
	job_queue_t queue;
} tsp_t;

typedef tsp_t * tsp_t_pointer;

typedef struct {
	tsp_t *tsp;
	int num_worker;
} tsp_worker_par_t;

tsp_t_pointer init_tsp(int partition, int n_partitions, int n_workers, int n_towns, int seed);
void free_tsp(tsp_t_pointer tsp);
void tsp (tsp_t_pointer tsp, int hops, int len, path_t *path, unsigned long *cuts, int num_worker);
void generate_jobs (tsp_t_pointer tsp);
void *worker (void *tsp_worker_par);
inline int tsp_get_shortest_path (tsp_t_pointer tsp);
inline int tsp_update_minimum_distance(tsp_t_pointer tsp, int length);

//callback
extern void new_minimun_distance_found(tsp_t_pointer tsp);

#endif
