#include <stdio.h>
#include <limits.h>
#include "tsp.h"

static distance_matrix_t *distance;
static job_queue_t *queue;
static int max_hops;

static struct {
	int distance;
	MUTEX_CREATE(mutex);
} minimun_distance;


void init_tsp(distance_matrix_t *distance_matrix, job_queue_t *q, int nb_workers, int n_towns) {
	int total;
	minimun_distance.distance = INT_MAX;
	MUTEX_INIT(minimun_distance.mutex);
	distance = distance_matrix;
	queue = q;
	max_hops = 0;
	total = 1;
	while (total < MIN_JOBS_THREAD * nb_workers && max_hops < n_towns) {
		max_hops++;
		total *= n_towns - max_hops;
	}
	max_hops++;
	LOG("MAX_HOPS %d\n", max_hops);
}

int present (int city, int hops, path_t *path) {
	unsigned int i;
	for (i = 0; i < hops; i++)
		if ((*path)[i] == city) return 1;
	return 0;
}

void tsp (int hops, int len, path_t *path, unsigned long *cuts, int num_worker) {
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
				LOG ("%2d ", (*path)[i]);
			LOG ("\n");
		}
		MUTEX_UNLOCK(minimun_distance.mutex);
	} else {
		int city, me, dist;
		me = (*path)[hops - 1];
		for (i = 0; i < distance->n_towns; i++) {
			city = distance->info[me][i].to_city;
			if (!present (city, hops, path)) {
				(*path)[hops] = city;
				dist = distance->info[me][i].dist;
				tsp (hops + 1, len + dist, path, cuts, num_worker);
			}
		}
	}
}

void distributor (int hops, int len, path_t *path) {
	job_t j;
	int i;	
	if (hops == max_hops) {
		j.len = len;
		for (i = 0; i < hops; i++)
			j.path[i] = (*path)[i];
		add_job (queue, j);
	} else {
		int me, city, dist;
		me = (*path)[hops - 1];
		for (i = 0; i < distance->n_towns; i++) {
			city = distance->info[me][i].to_city;
			if (!present(city,hops, path)) {
				(*path)[hops] = city;
				dist = distance->info[me][i].dist;
				distributor (hops + 1, len + dist, path);       
			}
		}
	}
}

void generate_jobs () {
	path_t path;
	path [0] = 0;
	distributor (1, 0, &path);
}

void *worker (void *num_worker_par) {
	long num_worker = (long)num_worker_par;
	int jobcount = 0;
	job_t job;
	unsigned long cuts = 0;

	while (get_job (queue, &job)) {
		jobcount++;
		tsp (max_hops, job.len, &job.path, &cuts, num_worker);
	}

	LOG ("Worker [%3lu] terminates, %4d jobs done with %16lu cuts.\n", num_worker, jobcount, cuts);
	return (void *) 0;
}

void tsp_log_shortest_path (void) {
	LOG ("Shortest path len = %d\n", minimun_distance.distance);
}