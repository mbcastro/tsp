#include <stdio.h>
#include <limits.h>
#include "tsp.h"

static distance_matrix_t *distance;
static job_queue_t *queue;

static struct {
	int distance;
	MUTEX_CREATE(mutex);
} minimun_distance;


void init_tsp(distance_matrix_t *distance_matrix, job_queue_t *q) {
	minimun_distance.distance = INT_MAX;
	MUTEX_INIT(minimun_distance.mutex);
	distance = distance_matrix;
	queue = q;
}

int present (int city, int hops, path_t *path) {
	unsigned int i;
	for (i = 0; i < hops; i++)
		if ((*path)[i] == city) return 1;
	return 0;
}

void tsp (int hops, int len, path_t *path_ptr, int *cuts, int num_worker) {
	int i;
	if (len >= minimun_distance.distance) {
		(*cuts)++;
		return;
	}
	if (hops == distance->n_towns) {
		MUTEX_LOCK(minimun_distance.mutex);
		if (len < minimun_distance.distance) {
			minimun_distance.distance = len;
			LOG ("worker[%d] finds path len = %3d :", num_worker, len);
			for (i = 0; i < distance->n_towns; i++)
				LOG ("%2d ", (*path_ptr)[i]);
			LOG ("\n");
		}
		MUTEX_UNLOCK(minimun_distance.mutex);
	} else {
		int city, me, dist;
		me = (*path_ptr) [hops - 1];
		for (i = 0; i < distance->n_towns; i++) {
			city = distance->to_city[me][i];
			if (!present (city, hops, path_ptr)) {
				(*path_ptr)[hops] = city;
				dist = distance->dist[me][i];
				tsp (hops + 1, len + dist, path_ptr, cuts, num_worker);
			}
		}
	}
}

void distributor (int hops, int len, path_t *path_ptr) {
	job_t j;
	int i;

	if (hops == MAX_HOPS) {
		j.len = len;
		for (i = 0; i < hops; i++)
			j.path[i] = (*path_ptr)[i];
		add_job (queue, j);
	} else {
		int me, city, dist;
		me = (*path_ptr) [hops - 1];
		for (i = 0; i < distance->n_towns; i++) {
			city = distance->to_city[me][i];
			if (!present(city,hops, path_ptr)) {
				(*path_ptr) [hops] = city;
				dist = distance->dist[me][i];
				distributor (hops + 1, len + dist, path_ptr);       
			}
		}
	} 
}

void generate_jobs (void) {
	path_t path;
	path [0] = 0;
	distributor (1, 0, &path); 
}

void *worker (void *num_worker_par) {
	long num_worker = (long)num_worker_par;
	int jobcount = 0;
	job_t job;
	int cuts = 0;

	LOG ("Worker [%2lu] starting \n", num_worker);

	while (get_job (queue, &job)) {
		jobcount++;
		tsp (MAX_HOPS, job.len, &job.path, &cuts, num_worker);
	}

	LOG ("Worker [%2lu] terminates, %4d jobs done with %4d cuts.\n", num_worker, jobcount, cuts);
	return (void *) 0;
}