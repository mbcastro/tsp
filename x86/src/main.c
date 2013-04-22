#include <stdio.h>
#include <stdlib.h>

#include "exec.h"

struct execution_parameters {
	int partition;
	int nb_partitions;
	int nb_threads;
	int nb_cities;
	int seed;
};

void *spawn_worker(void* params) {
	struct execution_parameters *p = (struct execution_parameters *)params;
	start_execution(p->partition, p->nb_partitions, p->nb_threads, p->nb_cities, p->seed);
	free(params);
	return NULL;
}

pthread_t *spawn (int partition, int nb_partitions, int nb_threads, int nb_cities, int seed) {
	pthread_t *tid = (pthread_t *)malloc (sizeof(pthread_t));
	struct execution_parameters *params = (struct execution_parameters*) malloc (sizeof(struct execution_parameters));
	params->partition = partition;
	params->nb_partitions = nb_partitions;
	params->nb_threads = nb_threads;
	params->nb_cities = nb_cities;
	params->seed = seed;
	pthread_create (tid, NULL, spawn_worker, params);
	return tid;	
}


int main (int argc, char **argv) {
	int i;

	CHECK_PAGE_SIZE();
	if (argc != 5) {
		fprintf (stderr, "Usage: %s <nb_threads> <nb_cities> <seed> <nb_partitions>\n", argv[0]);
		return 1;
	}

	int nb_threads = atoi(argv[1]);
	int nb_cities = atoi(argv[2]);
	int seed = atoi(argv[3]);
	int nb_partitions = atoi(argv[4]);
	
	pthread_t **tids = (pthread_t **) malloc (sizeof(pthread_t *) * nb_partitions);
	assert (tids != NULL);	
	for (i = 0; i < nb_partitions; i++)
		tids[i] = spawn(i, nb_partitions, nb_threads, nb_cities, seed);
	for (i = 0; i < nb_partitions; i++) {
		pthread_join (*(tids[i]), NULL);
		free(tids[i]);
	}
	free (tids);

	return 0;
}

void new_minimun_distance_found (int num_worker, int length) {
}