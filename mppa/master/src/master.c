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
static int clusters;
static portal_t *min_master_to_slave_portal;

void callback_slave_to_master (mppa_sigval_t sigval) {	
	int i, local_min, old_min, status;
	old_min = min;
	local_min = min;	
	MUTEX_LOCK(min_lock);
	for (i = 0; i < slave_comm_buffer_size; i++)
		local_min = (slave_comm_buffer[i] < local_min) ? slave_comm_buffer[i] : local_min;
	min = (local_min < min) ? local_min : min;
	MUTEX_UNLOCK(min_lock);
	printf("Recebi um callback. Min vec: ");

	for(i = 0; i <  slave_comm_buffer_size; i++)
		if(slave_comm_buffer[i] != INT_MAX)
			printf("%d, ", slave_comm_buffer[i]);
	printf("\n");

	if (old_min > min) {
		for (i = 0; i < clusters; i++) {		
			status = mppa_ioctl(min_master_to_slave_portal->file_descriptor, MPPA_TX_SET_RX_RANK, i);
		 	assert(status == 0);
			status = mppa_pwrite(min_master_to_slave_portal->file_descriptor, &min, sizeof(int), 0);
	   		assert(status > 0);
		}
	}
}

int main (int argc, const char **argv) {
	int rank, threads, status = 0, i;
	int pid;

	threads = atoi(argv[1]);
	clusters = atoi(argv[4]);
	slave_comm_buffer_size = clusters * threads;
	slave_comm_buffer = (int*)malloc(sizeof(int) * slave_comm_buffer_size);
	for (i = 0; i < slave_comm_buffer_size; i++) slave_comm_buffer[i] = INT_MAX;
	MUTEX_INIT(min_lock);

	printf("Starting execution. Towns = %s, Seed = %s -- Clusters = %d, Threads/Cluster %d\n", 
		argv[2], argv[3], clusters, threads);

    unsigned long start = get_time();

	portal_t *slave_to_master_portal = create_read_portal (MINIMUM_SLAVE_TO_MASTER, slave_comm_buffer, slave_comm_buffer_size * sizeof(int), 0, callback_slave_to_master);
	min_master_to_slave_portal = create_write_portal (MINIMUM_MASTER_TO_SLAVES, 0, clusters - 1);

  	for (rank = 0; rank < clusters; rank++) {
		pid = mppa_spawn(rank, NULL, "tsp_lock_mppa_slave", argv + 1, NULL);
		assert(pid >= 0);
	}
	
	close_portal(slave_to_master_portal);
	close_portal(min_master_to_slave_portal);

    for (rank = 0; rank < clusters; rank++) {
		status = 0;
	   	if ((status = mppa_waitpid(rank, &status, 0)) < 0) {
	    	printf("[I/O] Waitpid on cluster %d failed.\n", rank);
	     	mppa_exit(status);
	   	}
	}

	int local_min = INT_MAX;
	for (i = 0; i < slave_comm_buffer_size; i++)
		local_min = (slave_comm_buffer[i] < local_min) ? slave_comm_buffer[i] : local_min;
	min = (local_min < min) ? local_min : min;

    printf ("Execution time: %lu\n", diff_time(start, get_time()));
	printf("Shortest path length: %d\n", min);

	free(slave_comm_buffer);

	mppa_exit(0);
	return 0;
}

void new_minimun_distance_found (int num_worker, int lenght) {
	printf("SHOULD NOT BE HERE!!!\n");
}
