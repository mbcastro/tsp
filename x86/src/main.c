#include <stdio.h>
#include <stdlib.h>

#include "exec.h"

int main (int argc, char **argv) {

	execution_info_t exec_info;
	if (argc != 4) {
		fprintf (stderr, "Usage: %s <nb_threads > <ncities> <seed> \n",argv[0]);
		return 1;
	}

	CHECK_PAGE_SIZE();
	exec_info = start_execution(0, 1, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	
	print_execution_info(exec_info);

	return 0;
}

void new_minimun_distance_found (int num_worker, int length) {
}