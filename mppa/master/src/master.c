#include <stdio.h>
#include <stdlib.h>
#include <mppa/osconfig.h>
#include <assert.h>

#include "tsp_mppa.h"
#include "exec.h"
#include "timer.h"


static int *comm_buffer;
static int clusters;
static broadcast_t *broad;

void callback_master (mppa_sigval_t sigval);

int main (int argc, const char **argv) {
	int rank, threads, status = 0, i;
	int pid;

	threads = atoi(argv[1]);
	clusters = atoi(argv[4]);
	printf("Starting execution. Towns = %s, Seed = %s -- Clusters = %d, Threads/Cluster %d\n", 
		argv[2], argv[3], clusters, threads);

	int comm_buffer_size = (clusters + 1) * sizeof (int);
	comm_buffer = (int *) malloc(comm_buffer_size);
	for (i = 0; i <= clusters; i++) 
		comm_buffer[i] = INT_MAX;
	
    unsigned long start = get_time();	

	barrier_t *sync_barrier = create_master_barrier (BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE, clusters);
	broad = create_broadcast (clusters, BROADCAST_MASK, comm_buffer, comm_buffer_size, TRUE, callback_master);

  	for (rank = 0; rank < clusters; rank++) {
		pid = mppa_spawn(rank, NULL, "tsp_lock_mppa_slave", argv + 1, NULL);
		assert(pid >= 0);
	}
	
	barrier_wait(sync_barrier); //init barrier
	barrier_wait(sync_barrier); //end barrier

    for (rank = 0; rank < clusters; rank++) {
		status = 0;
	   	if ((status = mppa_waitpid(rank, &status, 0)) < 0) {
	    	printf("[I/O] Waitpid on cluster %d failed.\n", rank);
	     	mppa_exit(status);
	   	}
	}

	int min = INT_MAX;
	for (i = 0; i < clusters; i++)
		min = (comm_buffer[i] < min) ? comm_buffer[i] : min;

    printf ("Execution time: %lu\n", diff_time(start, get_time()));
	printf("Shortest path length: %d\n", min);

	close_barrier(sync_barrier);
	close_broadcast(broad);
	free(comm_buffer);

	mppa_exit(0);
	return 0;
}

void new_minimun_distance_found (int num_worker, int lenght) {
	printf("SHOULD NOT BE HERE!!!\n");
}

void callback_master (mppa_sigval_t sigval) {	
	int i;
	printf("Recebi um callback. Min vec: ");
	for(i = 0; i < clusters; i++)
		if(comm_buffer[i] != INT_MAX)
			printf("%d, ", comm_buffer[i]);
	printf("\n");
}
