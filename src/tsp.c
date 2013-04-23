#include <math.h>

#include "tsp.h"

void init_distance (tsp_t *tsp, int seed) {
	int n = tsp->distance->n_towns;	
	int x[n], y[n];
	int tempdist [n];	
	int i, j, k, city = 0;
	int dx, dy, tmp;

	RAND_STATE_T rand_state = RAND_INIT(seed);	

	for (i = 0; i < n; i++) {
		x[i] = RAND_NEXT(rand_state) % MAX_GRID_X;
		y[i] = RAND_NEXT(rand_state) % MAX_GRID_Y;
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
			tsp->distance->info[i][j].to_city = city;
			tsp->distance->info[i][j].dist = tmp;
		}
	}
}


tsp_t_pointer init_tsp(int partition, int nb_partitions, int nb_workers, int nb_towns, int seed) {
	int total;
	
	tsp_t_pointer tsp = (tsp_t_pointer) malloc (sizeof (tsp_t));
	assert(tsp != NULL);
	tsp->distance = (distance_matrix_t *) malloc (sizeof (distance_matrix_t));
	assert(tsp->distance != NULL);

	MUTEX_INIT(tsp->mutex);
	tsp->partition = partition;
	tsp->nb_partitions = nb_partitions;
	tsp->nb_workers = nb_workers;
	tsp->min_distance = INT_MAX;
	tsp->distance->n_towns = nb_towns;
	init_distance(tsp, seed);

	tsp->max_hops = 0;
	total = 1;
	while (total < MIN_JOBS_THREAD * nb_workers * nb_partitions && tsp->max_hops < nb_towns - 1) {
		tsp->max_hops++;
		total *= nb_towns - tsp->max_hops;
	}
	tsp->max_hops++;

	if (partition == 0) {
		LOG("MAX_HOPS %d\n", tsp->max_hops);
		LOG("NB_TASKS %d\n", total);
	}
	
	unsigned long queue_size = total / nb_partitions + total % nb_partitions;
	init_queue(&tsp->queue, queue_size);

	return tsp;
}

void free_tsp(tsp_t_pointer tsp) {
	free_queue(&tsp->queue);
	free(tsp->distance);
	free(tsp);
}

int present (int city, int hops, path_t *path) {
	unsigned int i;
	for (i = 0; i < hops; i++)
		if ((*path)[i] == city) return 1;
	return 0;
}

void tsp (tsp_t_pointer tsp_par, int hops, int len, path_t *path, unsigned long *cuts, int num_worker) {
	int i;
	if (len >= tsp_get_shortest_path(tsp_par)) {
		(*cuts)++;
		return;
	}
	if (hops == tsp_par->distance->n_towns) {
		if (tsp_update_minimum_distance(tsp_par, len)) {
			new_minimun_distance_found(tsp_par);
			LOG ("worker[%d] finds path len = %3d :", num_worker, len);
			for (i = 0; i < tsp_par->distance->n_towns; i++)
				LOG ("%2d ", (*path)[i]);
			LOG ("\n");
		}
	} else {
		int city, me, dist;
		me = (*path)[hops - 1];
		for (i = 0; i < tsp_par->distance->n_towns; i++) {
			city = tsp_par->distance->info[me][i].to_city;
			if (!present (city, hops, path)) {
				(*path)[hops] = city;
				dist = tsp_par->distance->info[me][i].dist;
				tsp (tsp_par, hops + 1, len + dist, path, cuts, num_worker);
			}
		}
	}
}

void distributor (tsp_t_pointer tsp_par, int hops, int len, path_t *path, int *job_index) {
	job_t j;
	int i;	
	if (hops == tsp_par->max_hops) {
		if ((*job_index) % tsp_par->nb_partitions == tsp_par->partition) {
			j.len = len;
			for (i = 0; i < hops; i++)
				j.path[i] = (*path)[i];			
			add_job (&tsp_par->queue, j);
		}
		(*job_index)++;
	} else {
		int me, city, dist;
		me = (*path)[hops - 1];
		for (i = 0; i < tsp_par->distance->n_towns; i++) {
			city = tsp_par->distance->info[me][i].to_city;
			if (!present(city, hops, path)) {
				(*path)[hops] = city;
				dist = tsp_par->distance->info[me][i].dist;
				distributor (tsp_par, hops + 1, len + dist, path, job_index);       
			}
		}
	}
}

void generate_jobs (tsp_t_pointer tsp) {
	int i, job_count = 0;
	path_t path;	
	LOG("Task generation starting...\n");
	path [0] = 0;
	distributor (tsp, 1, 0, &path, &job_count);
	close_queue(&tsp->queue);
	LOG("Task generation complete.\n");
	for (i = 0; i < tsp->nb_partitions; i++)
		COND_VAR_SIGNAL(tsp->queue.cond_var);
}

void *worker (void *pars) {
	tsp_worker_par_t *p = (tsp_worker_par_t *)pars;

	int jobcount = 0;
	job_t job;
	unsigned long cuts = 0;
	int finished = 0;

	while (!finished)
		switch (get_job (&p->tsp->queue, &job)) {
			case QUEUE_OK:
				jobcount++;
				tsp (p->tsp, p->tsp->max_hops, job.len, &job.path, &cuts, p->num_worker);
				break;
			case QUEUE_CLOSED:
				finished = 1;
				break;
		}

	LOG ("Worker [%3d] terminates, %4d jobs done with %16lu cuts.\n", p->num_worker, jobcount, cuts);
	free(pars);
	return NULL;
}

void tsp_log_shortest_path (tsp_t *tsp) {
	LOG ("Shortest path len = %d\n", tsp_get_shortest_path(tsp));
}

inline int tsp_get_shortest_path (tsp_t *tsp) {
#ifdef NO_CACHE_COHERENCE
	__builtin_k1_dflush();
#endif
	return tsp->min_distance;
}

int tsp_update_minimum_distance (tsp_t_pointer tsp, int new_distance) {
	int min_updated = 0;
	MUTEX_LOCK(tsp->mutex);
	if (new_distance < tsp->min_distance) {
		tsp->min_distance = new_distance;
		min_updated = 1;
	}
	MUTEX_UNLOCK(tsp->mutex);
	return min_updated;
}