#include <stdio.h>
#include <stdlib.h>
#include <mppa/osconfig.h>
#include <assert.h>

#include "tsp_mppa.h"
#include "exec.h"
#include "timer.h"


static MUTEX_CREATE(min_lock);
static int min = INT_MAX;
static int slave_comm_buffer_size;
static int *slave_comm_buffer;

void callback_slave_to_master (mppa_sigval_t sigval) {	
	int i, local_min;
	local_min = min;	
	for (i = 0; i < slave_comm_buffer_size; i++)
		local_min = (slave_comm_buffer[i] < local_min) ? slave_comm_buffer[i] : local_min;
	MUTEX_LOCK(min_lock);
	min = (local_min < min) ? local_min : min;
	MUTEX_UNLOCK(min_lock);
	printf("Recebi um callback. Min: %d\n", min);
}

int main (int argc, const char **argv) {
	int rank, clusters, threads, status = 0, i;
	int pid, min2;

	threads = atoi(argv[1]);
	clusters = atoi(argv[4]);
	slave_comm_buffer_size = clusters * threads;
	slave_comm_buffer = (int*)malloc(sizeof(int) * slave_comm_buffer_size);
	for (i = 0; i < slave_comm_buffer_size; i++) slave_comm_buffer[i] = INT_MAX;
	MUTEX_INIT(min_lock);
	execution_info_t exec_info[clusters];

	printf("Starting execution. Towns = %s, Seed = %s -- Clusters = %d, Threads/Cluster %d\n", 
		argv[2], argv[3], clusters, threads);

    unsigned long start = get_time();

	portal_t *results_portal = create_read_portal (RESULTS_GATHERING, exec_info, sizeof(execution_info_t) * clusters, clusters, NULL);
	portal_t *slave_to_master_portal = create_read_portal (MINIMUM_SLAVE_TO_MASTER, slave_comm_buffer, slave_comm_buffer_size * sizeof(int), 0, callback_slave_to_master);

  	for (rank = 0; rank < clusters; rank++) {
		pid = mppa_spawn(rank, NULL, "tsp_lock_mppa_slave", argv + 1, NULL);
		assert(pid >= 0);
	}
	
	status = mppa_aio_wait(&results_portal->portal);
	assert(status > 0);

	close_portal(results_portal);
	close_portal(slave_to_master_portal);

    min2 = INT_MAX;
    for (rank = 0; rank < clusters; rank++) {
		status = 0;
	   	if ((status = mppa_waitpid(rank, &status, 0)) < 0) {
	    	printf("[I/O] Waitpid on cluster %d failed.\n", rank);
	     	mppa_exit(status);
	   	}
	   	print_execution_info(exec_info[rank]);
	   	if (exec_info[rank].shortest_path_length < min2)
	   		min2 = exec_info[rank].shortest_path_length;
	} 
    printf ("Execution time: %lu\n", diff_time(start, get_time()));
	printf("Shortest path length: %d e o outro %d\n", min2, min);

	free(slave_comm_buffer);

	mppa_exit(0);
	return 0;
}

void new_minimun_distance_found (int num_worker, int lenght) {
	printf("SHOULD NOT BE HERE!!!\n");
}
