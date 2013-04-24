#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "exec.h"
#include "timer.h"

static tsp_t_pointer tsp_instance;

static int min_distance;
MUTEX_CREATE(min_lock, static);

void run_tsp (int nb_threads, int nb_towns, int seed, int nb_partitions) {
	unsigned long start = get_time ();
	min_distance = INT_MAX;
 	tsp_instance = init_execution (0, nb_partitions, nb_threads, nb_towns, seed);
	start_execution (tsp_instance);
	end_execution (tsp_instance);

  	unsigned long exec_time = diff_time(start, get_time());
	printf ("%lu\t%d\t%d\t%d\t%d\t%d\n", 
		exec_time, min_distance, nb_threads, nb_towns, seed, nb_partitions);
}


int main (int argc, char **argv) {

	CHECK_PAGE_SIZE();
	if (argc != 5 && argc != 6) {
		fprintf (stderr, "Usage: %s <nb_threads> <nb_towns> <seed> <nb_partitions=1> [nb_executions]\n", argv[0]);
		return 1;
	}
	
	init_time();
	MUTEX_INIT(min_lock);

	int *nb_threads = par_parse (argv[1]);
	assert(nb_threads);
	int *nb_towns = par_parse (argv[2]);
	assert(nb_towns);
	int seed = atoi(argv[3]);
	int nb_partitions = atoi (argv[4]);
	assert(nb_partitions == 1);
	int nb_executions = (argc == 6) ? atoi(argv[5]) : 1;

	int execution, town, thread;
	town = 0;
	while (nb_towns[town] != 0) {		
		thread = 0;
		while (nb_threads[thread] != 0) {
			assert (nb_threads[thread] > 0);
			assert (nb_towns[town] <= MAX_TOWNS);			
			for (execution = 0; execution < nb_executions; execution++)
				run_tsp(nb_threads[thread], nb_towns[town], seed, nb_partitions);
			thread++;
		}
		town++;
	}

	return 0;
}

void new_minimun_distance_found(tsp_t_pointer tsp) {
	int min = tsp_get_shortest_path(tsp);
	if (min_distance > min) {
		MUTEX_LOCK(min_lock);
		if (min_distance > min)
			min_distance = min;
		MUTEX_UNLOCK(min_lock);
	}
}


