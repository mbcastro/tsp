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


void init_tsp(distance_matrix_t *distance_matrix, job_queue_t *q, int partition, int n_partitions, int n_workers, int n_towns) {
	int total;
	
	minimun_distance.distance = INT_MAX;
	MUTEX_INIT(minimun_distance.mutex);
	distance = distance_matrix;
	queue = q;
	max_hops = 0;
	total = 1;
	while (total < MIN_JOBS_THREAD * n_workers * n_partitions && max_hops < n_towns - 1) {
		max_hops++;
		total *= n_towns - max_hops;
	}
	max_hops++;

	if (partition == 0) {
		LOG("MAX_HOPS %d\n", max_hops);
		LOG("NB_TASKS %d\n", total);
	}
	
	unsigned long queue_size = total / n_partitions + total % n_partitions;
	init_queue(q, queue_size);

}

int present (int city, int hops, path_t *path) {
	unsigned int i;
	for (i = 0; i < hops; i++)
		if ((*path)[i] == city) return 1;
	return 0;
}

void tsp (int hops, int len, path_t *path, unsigned long *cuts, int num_worker) {
	int i;
	if (len >= tsp_get_shortest_path()) {
		(*cuts)++;
		return;
	}
	if (hops == distance->n_towns) {
		if (tsp_update_minimum_distance(len)) {
			new_minimun_distance_found(num_worker, len);
			LOG ("worker[%d] finds path len = %3d :", num_worker, len);
			for (i = 0; i < distance->n_towns; i++)
				LOG ("%2d ", (*path)[i]);
			LOG ("\n");
		}
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

void distributor (int hops, int len, path_t *path, const int partition, const int n_partitions, int *job_index) {
	job_t j;
	int i;	
	if (hops == max_hops) {
		if ((*job_index) % n_partitions == partition) {
			j.len = len;
			for (i = 0; i < hops; i++)
				j.path[i] = (*path)[i];			
			add_job (queue, j);
		}
		(*job_index)++;
	} else {
		int me, city, dist;
		me = (*path)[hops - 1];
		for (i = 0; i < distance->n_towns; i++) {
			city = distance->info[me][i].to_city;
			if (!present(city,hops, path)) {
				(*path)[hops] = city;
				dist = distance->info[me][i].dist;
				distributor (hops + 1, len + dist, path, partition, n_partitions, job_index);       
			}
		}
	}
}

void generate_jobs (const int partition, const int n_partitions) {
	int i, job_count = 0;
	path_t path;	
	LOG("Task generation starting...\n");
	path [0] = 0;
	distributor (1, 0, &path, partition, n_partitions, &job_count);
	close_queue(queue);
	LOG("Task generation complete.\n");
	for (i = 0; i < n_partitions; i++)
		COND_VAR_SIGNAL(queue->cond);
}

void *worker (void *num_worker_par) {

	long num_worker = (long)num_worker_par;
	int jobcount = 0;
	job_t job;
	unsigned long cuts = 0;
	int finished = 0;

	while (!finished)
		switch (get_job (queue, &job)) {
			case QUEUE_OK:
				jobcount++;
				tsp (max_hops, job.len, &job.path, &cuts, num_worker);
				break;
			case QUEUE_CLOSED:
				finished = 1;
				break;
		}

	LOG ("Worker [%3lu] terminates, %4d jobs done with %16lu cuts.\n", num_worker, jobcount, cuts);
	return (void *) 0;
}

void tsp_log_shortest_path (void) {
	LOG ("Shortest path len = %d\n", tsp_get_shortest_path());
}

inline int tsp_get_shortest_path (void) {
#ifdef NO_CACHE_COHERENCE
	__builtin_k1_dflush();
#endif
	return minimun_distance.distance;
}

int tsp_update_minimum_distance (int new_distance) {
	int min_updated = 0;
	MUTEX_LOCK(minimun_distance.mutex);
	if (new_distance < minimun_distance.distance) {
		minimun_distance.distance = new_distance;
		min_updated = 1;
	}
	MUTEX_UNLOCK(minimun_distance.mutex);
	return min_updated;
}