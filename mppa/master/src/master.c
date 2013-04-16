#include <stdio.h>
#include <stdlib.h>
#include <mppa/osconfig.h>
#include <mppaipc.h>
#include <assert.h>

#include "tsp_mppa.h"
#include "exec.h"
#include "timer.h"

int main (int argc, const char **argv) {
	int rank, clusters, status = 0;
	int results_portal_fd, pid, min;
	
	clusters = atoi(argv[4]);
	execution_info_t exec_info[clusters];

	printf("Starting execution. Towns = %s, Seed = %s, Rank = %d\n", argv[2], argv[3], __k1_get_cluster_id());

    unsigned long start = get_time();

   	results_portal_fd = mppa_open(RESULTS_GATHERING, O_RDONLY);
  	assert(results_portal_fd != -1);

  	mppa_aiocb_t results_portal[1] = {MPPA_AIOCB_INITIALIZER(results_portal_fd, exec_info, sizeof(execution_info_t) * clusters)};
  	mppa_aiocb_set_trigger(results_portal, clusters);
  	status = mppa_aio_read(results_portal);
  	assert(status == 0);

  	for (rank = 0; rank < clusters; rank++) {
		pid = mppa_spawn(rank, NULL, "tsp_lock_mppa_slave", argv + 1, NULL);
		assert(pid >= 0);
	}
	
	status = mppa_aio_wait(results_portal);
	assert(status > 0);


    mppa_close(results_portal_fd);

    min = INT_MAX;
    for (rank = 0; rank < clusters; rank++) {
		status = 0;
	   	if ((status = mppa_waitpid(rank, &status, 0)) < 0) {
	    	printf("[I/O] Waitpid on cluster %d failed.\n", rank);
	     	mppa_exit(status);
	   	}
	   	print_execution_info(exec_info[rank]);
	   	if (exec_info[rank].shortest_path_length < min)
	   		min = exec_info[rank].shortest_path_length;
	} 
    printf ("Execution time: %lu\n", diff_time(start, get_time()));
	printf("Shortest path length: %d\n", min);

	mppa_exit(0);
	return 0;
}
