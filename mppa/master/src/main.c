#include <stdio.h>
#include <stdlib.h>
#include <mppa/osconfig.h>
#include <mppaipc.h>

#include "exec.h"

int main (int argc, char **argv) {
	CHECK_PAGE_SIZE();
	start_execution(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

	mppa_exit(0);
	return 0;
}
