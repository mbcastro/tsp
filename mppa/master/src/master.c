#include <mppa/osconfig.h>

#include "tsp_mppa.h"

static int *comm_buffer;
static int clusters;
static broadcast_t *broad;

void callback_master (mppa_sigval_t sigval);

void run_tsp (int nb_threads, int nb_towns, int seed, int clusters_p) {
  
	int rank, status = 0, i;
	int pid;

	clusters = clusters_p;

    uint64_t start = mppa_get_time();

	LOG("Starting execution. Towns = %d, Seed = %d -- Clusters = %d, Threads/Cluster %d\n", 
		nb_towns, seed, clusters, nb_threads);

	int comm_buffer_size = (clusters + 1) * sizeof (int);
	comm_buffer = (int *) malloc(comm_buffer_size);
	for (i = 0; i <= clusters; i++) 
		comm_buffer[i] = INT_MAX;

	LOG("Creating communication portals\n");
	barrier_t *sync_barrier = create_master_barrier (BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE, clusters);
	broad = create_broadcast (clusters, BROADCAST_MASK, comm_buffer, comm_buffer_size, TRUE, callback_master);


	char **argv = (char**) malloc(sizeof (char*) * 4);
	for (i = 0; i < 4; i++)
		argv[i] = (char*) malloc (sizeof (char) * 10);

	sprintf(argv[0], "%d", nb_threads); 
	sprintf(argv[1], "%d", nb_towns);
	sprintf(argv[2], "%d", seed);
	sprintf(argv[3], "%d", clusters);

	LOG("Spawning clusters.\n");
  	for (rank = 0; rank < clusters; rank++) {
		pid = mppa_spawn(rank, NULL, "tsp_lock_mppa_slave", (const char **)argv, NULL);
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

	close_barrier(sync_barrier);
	close_broadcast(broad);
	free(comm_buffer);
	for (i = 0; i < 4; i++)
		free(argv[i]);
	free(argv);

   	uint64_t exec_time = mppa_diff_time(start, mppa_get_time());
	printf ("%llu\t\t%d\t%d\t%d\t%d\t%d\n", exec_time, min, nb_threads, nb_towns, seed, clusters);

}

int main (int argc, const char **argv) {
	int *nb_threads, *nb_towns, seed, *nb_partitions, nb_executions;
	int town, partition, thread, execution;

	if (argc != 5 && argc != 6) {
		fprintf (stderr, "Usage: %s <nb_threads> <nb_towns> <seed> <nb_partitions> [nb_executions]\n", argv[0]);
		return 1;
	}

	mppa_init_time();

	nb_threads = par_parse (argv[1]);
	assert(nb_threads);
	nb_towns = par_parse (argv[2]);
	assert(nb_towns);
	seed = atoi(argv[3]);
	nb_partitions = par_parse (argv[4]);
	assert(nb_partitions);

	if(argc == 6) {
		nb_executions = atoi(argv[5]);
		assert(nb_executions > 0);
	}
	else 
		nb_executions = 1;

	town = 0;
	while (nb_towns[town] != 0) {
		partition = 0;
		while (nb_partitions[partition] != 0) {
			thread = 0;
			while (nb_threads[thread] != 0) {
				assert (nb_threads[thread] > 0 && nb_threads[thread] <= MAX_THREADS_PER_CLUSTER);
				assert (nb_towns[town] <= MAX_TOWNS);
				assert (nb_partitions[partition] > 0 && nb_partitions[partition] <= MAX_CLUSTERS);
				for (execution = 0; execution < nb_executions; execution++)
					run_tsp(nb_threads[thread], nb_towns[town], seed, nb_partitions[partition]);
				thread++;
			}
			partition++;
		}
		town++;
	}

	free(nb_towns);
	free(nb_threads);
	free(nb_partitions);

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
