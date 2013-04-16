#include <fcntl.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tsp_mppa.h"
#include "exec.h"
#include "timer.h"

int main(int argc, char **argv) {	
	int status, rank, clusters;
	execution_info_t exec_info;

	int results_portal_fd = mppa_open(RESULTS_GATHERING, O_WRONLY);
	assert(results_portal_fd != -1);

	clusters = atoi(argv[3]);
	rank = __k1_get_cluster_id();
	LOG("Cluster %d/%d starting.\n", rank, clusters);

	exec_info = start_execution(rank, clusters, atoi(argv[0]), atoi(argv[1]), atoi(argv[2]));
	
	status = mppa_pwrite(results_portal_fd, &exec_info, sizeof(execution_info_t), rank * sizeof(execution_info_t));
   	assert(status > 0);
    	
   	mppa_close(results_portal_fd);

	mppa_exit(0);
	return 0;
}