#include <stdio.h>
#include <stdlib.h>

#include "exec.h"

int main (int argc, char **argv) {

	if (argc != 4) {
		fprintf (stderr, "Usage: %s <nb_threads > <ncities> <seed> \n",argv[0]);
		return 1;
	}

	CHECK_PAGE_SIZE();
	start_execution(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	
	return 0;
}
