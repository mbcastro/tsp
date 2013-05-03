#include <mppa/osconfig.h>

#include "tsp_mppa.h"
#include "common_main.h"

static int *comm_buffer;
static int clusters;
static broadcast_t *broad;

void callback_master (mppa_sigval_t sigval);

int main (int argc, char **argv) {
	struct main_pars pars = init_main_pars(argc, argv);

	mppa_init_time();

	run_main(pars);	
	free_main(pars);

	mppa_exit(0);
	
	return 0;
}

void run_tsp (int nb_threads, int nb_towns, int seed, int nb_clusters, char* machine) {
	assert (nb_threads <= MAX_THREADS_PER_CLUSTER);
	assert (nb_clusters <= MAX_CLUSTERS);

	int rank, status = 0, i;
	int pid;
	int nb_partitions = nb_clusters * PARTITIONS_PER_CLUSTER;
	int finished_clusters = 0;
	int next_partition = 0;

	LOG ("nb_clusters = %3d nb_partitions = %3d nb_threads = %3d nb_towns = %3d seed = %d \n", 
		nb_clusters, nb_partitions, nb_threads, nb_towns, seed);

    uint64_t start = mppa_get_time();

	int comm_buffer_size = (nb_clusters + 1) * sizeof (int);
	comm_buffer = (int *) malloc(comm_buffer_size);
	for (i = 0; i <= nb_clusters; i++) 
		comm_buffer[i] = INT_MAX;

	LOG("Creating communication portals\n");
	barrier_t *sync_barrier = mppa_create_master_barrier (BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE, nb_clusters);
	barrier_par_t barrier;
	barrier.void_t = sync_barrier;
	
	broad = mppa_create_broadcast (nb_clusters, BROADCAST_MASK, comm_buffer, comm_buffer_size, TRUE, callback_master);

	rqueue_t *rqueue_partition_request = mppa_create_read_rqueue(2 * sizeof(int), 128, 70, "[0..15]", 71);
	
	rqueue_t **rqueue_partition_response = (rqueue_t **) malloc (nb_clusters * sizeof(rqueue_t *));
	for(i = 0; i < nb_clusters; i++)
		rqueue_partition_response[i] = mppa_create_write_rqueue(sizeof(partition_interval_t), i, i + 72, "128", i + 72 + MAX_CLUSTERS);

	char **argv = (char**) malloc(sizeof (char*) * 4);
	for (i = 0; i < 4; i++)
		argv[i] = (char*) malloc (sizeof (char) * 10);

	sprintf(argv[0], "%d", nb_threads); 
	sprintf(argv[1], "%d", nb_towns);
	sprintf(argv[2], "%d", seed);
	sprintf(argv[3], "%d", nb_clusters);

	LOG("Spawning nb_clusters.\n");
  	for (rank = 0; rank < nb_clusters; rank++) {
		pid = mppa_spawn(rank, NULL, "tsp_lock_mppa_slave", (const char **)argv, NULL);
		assert(pid >= 0);
	}
	
	wait_barrier (barrier); //init barrier

	//TO SOLVE THE BUG!!!
	mppa_init_read_rqueue(rqueue_partition_request, nb_clusters);

	//Manage partition requests
	while(finished_clusters < nb_clusters) {
		int from[2];
		partition_interval_t partition_interval;

		LOG("Waiting requests\n");
		mppa_read_rqueue (rqueue_partition_request, &from, 2 * sizeof(int));
		LOG("Request received\n");
		
		partition_interval = get_next_partition_default_impl(nb_partitions, nb_clusters, &next_partition, from[1]);
		
		if(partition_interval.start == -1) 
			finished_clusters++;

		LOG("Sending partition to cluster %d\n", from[0]);
		mppa_write_rqueue (rqueue_partition_response[from[0]], &partition_interval, sizeof(partition_interval_t));
		LOG("Sent partition to cluster %d\n", from[0]);
	}

	wait_barrier (barrier); //end barrier

    for (rank = 0; rank < nb_clusters; rank++) {
		status = 0;
	   	if ((status = mppa_waitpid(rank, &status, 0)) < 0) {
	    	printf("[I/O] Waitpid on cluster %d failed.\n", rank);
	     	mppa_exit(status);
	   	}
	}

	int min = INT_MAX;
	for (i = 0; i < nb_clusters; i++)
		min = (comm_buffer[i] < min) ? comm_buffer[i] : min;

	mppa_close_barrier(sync_barrier);
	mppa_close_broadcast(broad);

	mppa_close_rqueue(rqueue_partition_request);
	for(i = 0; i < nb_clusters; i++)
		mppa_close_rqueue(rqueue_partition_response[i]);

	free(comm_buffer);
	
	for (i = 0; i < 4; i++)
		free(argv[i]);
	free(argv);

   	uint64_t exec_time = mppa_diff_time(start, mppa_get_time());
   	printf ("%llu\t%d\t%d\t%d\t%d\t%d\t%d\n", 
		exec_time, min, nb_threads, nb_towns, seed, nb_clusters, nb_partitions);

}

void new_minimun_distance_found(tsp_t_pointer tsp) {
	printf("SHOULD NOT BE HERE!!!\n");
}

partition_interval_t get_next_partition(tsp_t_pointer tsp) {
	printf("SHOULD NOT BE HERE!!!\n");
	partition_interval_t dummy;
	dummy.start = -1;
	dummy.end = -1;	
	return dummy;
}

void callback_master (mppa_sigval_t sigval) {	
	int i;
	LOG("Received a callback. Min vector: ");
	for(i = 0; i < clusters; i++)
		if(comm_buffer[i] != INT_MAX)
			LOG("%d, ", comm_buffer[i]);
	LOG("\n");
}
