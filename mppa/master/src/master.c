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
	int rank, nb_threads, nb_towns, seed, status = 0, i;
	int pid;

	nb_threads = atoi(argv[1]);
	nb_towns = atoi(argv[2]);
	seed = atoi(argv[3]);
	clusters = atoi(argv[4]);

	LOG("Starting execution. Towns = %d, Seed = %d -- Clusters = %d, Threads/Cluster %d\n", 
		nb_towns, seed, clusters, nb_threads);

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

    unsigned long exec_time = diff_time(start, get_time());
	printf ("%lu\t%d\t%d\t%d\t%d\t%d\n", exec_time, min, nb_threads, nb_towns, seed, clusters);

	close_barrier(sync_barrier);
	close_broadcast(broad);
	free(comm_buffer);

	mppa_exit(0);
	return 0;
}

void new_minimun_distance_found(tsp_t_pointer tsp) {
	printf("SHOULD NOT BE HERE!!!\n");
}

void callback_master (mppa_sigval_t sigval) {	
	int i;
	LOG("Received a callback. Min vector: ");
	for(i = 0; i < clusters; i++)
		if(comm_buffer[i] != INT_MAX)
			LOG("%d, ", comm_buffer[i]);
	LOG("\n");
}
