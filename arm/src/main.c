#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "exec.h"
#include "timer.h"

static tsp_t_pointer tsp_instance;

static int min_distance = INT_MAX;
MUTEX_CREATE(min_lock, static);

int main (int argc, char **argv) {

	CHECK_PAGE_SIZE();
	if (argc != 5) {
		fprintf (stderr, "Usage: %s <nb_threads> <nb_towns> <seed> <nb_partitions=1>\n", argv[0]);
		return 1;
	}
	
	init_time();
	unsigned long start = get_time();
	
	MUTEX_INIT(min_lock);
	int nb_threads = atoi(argv[1]);
	int nb_towns =  atoi(argv[2]);
	int seed =  atoi(argv[3]);
	int nb_partitions =  atoi(argv[4]);
	assert(nb_partitions == 1);

 	tsp_instance = init_execution(0, nb_partitions, nb_threads, nb_towns, seed);
	start_execution(tsp_instance);
	end_execution(tsp_instance);

  	unsigned long exec_time = diff_time(start, get_time());
	printf ("%lu\t%d\t%d\t%d\t%d\t%d\n", 
		exec_time, min_distance, nb_threads, nb_towns, seed, nb_partitions);

	return 0;
}

void new_minimun_distance_found(tsp_t_pointer tsp) {
	if (min_distance > tsp->min_distance) {
		MUTEX_LOCK(min_lock);
		if (min_distance > tsp->min_distance)
			min_distance = tsp->min_distance;
		MUTEX_UNLOCK(min_lock);
	}
}


