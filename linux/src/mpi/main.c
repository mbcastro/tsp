#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sched.h>
#include <errno.h>
#include <mpi.h>

#include "cpu_affinity.h"
#include "common_main.h"

static tsp_t_pointer tsp;

static int next_partition;
MUTEX_CREATE(main_lock, static);

pthread_t *spawn (tsp_t_pointer *tsp, int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed, char* machine) {
	pthread_t *tid = (pthread_t *)malloc (sizeof(pthread_t));
	struct execution_parameters *params = (struct execution_parameters*) malloc (sizeof(struct execution_parameters));
	params->cluster = cluster_id;
	params->nb_clusters = nb_clusters;
	params->nb_partitions = nb_partitions;
	params->nb_threads = nb_threads;
	params->nb_towns = nb_towns;
	params->seed = seed;
	params->tsp = tsp;
	params->barrier_par = nb_clusters;

	int status = pthread_create (tid, NULL, spawn_worker, params);
	assert (status == 0);

	if (machine) {
		char **machine_sched = get_machine_sched(machine);
		cpu_set_t *cpu_set = mask_for_partition(cluster_id, machine_sched);
		status = pthread_setaffinity_np (*tid, sizeof(cpu_set_t), cpu_set);
		assert (status == 0);
		free(cpu_set);
	}

	return tid;	
}


int main (int argc, char **argv) {

	struct main_pars pars = init_main_pars(argc, argv);
	
	int provided_thread_level;
	MPI_Init_thread (&argc, &argv, MPI_THREAD_MULTIPLE, &provided_thread_level);
	LOG("provided_thread_level %d\n", provided_thread_level);
	assert(MPI_THREAD_MULTIPLE == provided_thread_level);

	int mpi_comm_size;
	MPI_Comm_size (MPI_COMM_WORLD, &mpi_comm_size);
	assert(pars.nb_clusters[0] == mpi_comm_size);
	
	run_main(pars);	
	
	MPI_Finalize();

	return 0;
}

void wait_barrier (int limit) {
	int status = MPI_Barrier(MPI_COMM_WORLD);
	assert(status == MPI_SUCCESS);
}
 
enum {
	MINIMUM_UPDATE,
	FINALIZE,
	PARTITION_REQUEST,
	PARTITION_RESPONSE,
	MSG_TO_MASTER
};

void broadcast (int new_minimum) {	
	if (new_minimum >= tsp->min_distance) return;

	int i;
	for (i = 1; i < tsp->nb_clusters; i++) {
		int return_status = MPI_Send(&new_minimum, 1, MPI_INT, i, MINIMUM_UPDATE, MPI_COMM_WORLD);
		assert(return_status == MPI_SUCCESS);
	}
	
	if (new_minimum != -1) {
		LOG ("MASTER Broadcasting new minimum\n");
		tsp_update_minimum_distance(tsp, new_minimum);
	} else
		LOG ("MASTER Broadcasting finalize\n");
}



void *spawn_comm_thread (void *params) {
	partition_interval_t partition_interval;
	int my_rank = *(int*)params;
	if (my_rank == 0) {
		while (1) {
			int buffer[2];
			int return_status;
			MPI_Status status;
			LOG("MASTER Waiting for message\n");
			return_status = MPI_Recv (buffer, 2, MPI_INT, MPI_ANY_SOURCE, MSG_TO_MASTER, MPI_COMM_WORLD, &status);
			assert(return_status == MPI_SUCCESS);
			LOG("MASTER Received message tag %d from %d option %d\n", status.MPI_TAG, status.MPI_SOURCE, buffer[0]);
			switch (buffer[0]) {
				case MINIMUM_UPDATE:
					LOG("MASTER: Received minimun update from %d new minimum: %d\n", status.MPI_SOURCE, buffer[1]);
					broadcast(buffer[1]);
					break;
				case PARTITION_REQUEST:					
					LOG("MASTER: Received request for partition interval from %d\n", status.MPI_SOURCE);
					MUTEX_LOCK(main_lock);
					partition_interval = get_next_partition_default_impl(tsp, &next_partition, buffer[1]);
					MUTEX_UNLOCK(main_lock);
					LOG("MASTER: Will send response to cluster %d [%d_%d] for partition interval\n", status.MPI_SOURCE, partition_interval.start, partition_interval.end);
					return_status = MPI_Send(
						&partition_interval, sizeof(partition_interval_t), MPI_BYTE, 
						status.MPI_SOURCE, PARTITION_RESPONSE, MPI_COMM_WORLD);
					assert(return_status == MPI_SUCCESS);
					LOG("MASTER: Sent response to  %d [%d_%d] for partition interval\n", status.MPI_SOURCE, partition_interval.start, partition_interval.end);
					break;
				case FINALIZE:
					LOG("MASTER: Received FINALIZE.\n");
					broadcast (-1);
					return NULL;
			}
		}
	} else {
		while (1) {
			int new_minimum;
			LOG ("Cluster %d Waiting for broadcast of new minimum\n", my_rank);
			MPI_Status status;
			int return_status = MPI_Recv (&new_minimum, 1, MPI_INT, 0, MINIMUM_UPDATE, MPI_COMM_WORLD, &status);
			assert(return_status == MPI_SUCCESS);			
			LOG ("Cluster %d Received broadcast of new minimum: %d\n", my_rank, new_minimum);
			if (new_minimum == -1)
				return NULL;
			else
				tsp_update_minimum_distance (tsp, new_minimum);
		}
	}
}

void run_tsp (int nb_threads, int nb_towns, int seed, int nb_clusters, char* machine) {

	unsigned long start = get_time();
	int nb_partitions = get_number_of_partitions(nb_clusters);

	next_partition = 0;
	
	int my_rank;
	MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);
	
	if (my_rank == 0)
		LOG ("nb_clusters = %3d nb_partitions = %3d nb_threads = %3d nb_towns = %3d seed = %d \n", 
			nb_clusters, nb_partitions, nb_threads, nb_towns, seed);
	
	pthread_t comm_thread;
	int status = pthread_create (&comm_thread, NULL, spawn_comm_thread, (void*)&my_rank);
	assert (status == 0);

	pthread_t *tid;
	
	tid = spawn(&tsp, my_rank, nb_clusters, nb_partitions, nb_threads, nb_towns, seed, machine);
	pthread_join (*tid, NULL);
	free(tid);

	LOG("Cluster %d - Execution terminated. Starting clean up\n", my_rank);

	if (my_rank == 0) {
		int buffer[2];
		buffer[0] = FINALIZE;
		LOG("MASTER Will send FINALIZE\n");
		status = MPI_Send (buffer, 2, MPI_INT, 0, MSG_TO_MASTER, MPI_COMM_WORLD);
		assert (status == MPI_SUCCESS);
	    unsigned long exec_time = diff_time(start, get_time());
		printf ("%lu\t%d\t%d\t%d\t%d\t%d\t%d", 
			exec_time, tsp->min_distance, nb_threads, nb_towns, seed, nb_clusters, nb_partitions);
	}
	pthread_join(comm_thread, NULL);
	tsp = NULL;

}

void new_minimun_distance_found(tsp_t_pointer tsp) {	
	int min = tsp_get_shortest_path(tsp);
	LOG("Cluster %d Will send the new minimum to master\n", tsp->cluster_id);
	int buffer[2];
	buffer[0] = MINIMUM_UPDATE;
	buffer[1] = min;
	int status = MPI_Send(buffer, 2, MPI_INT, 0, MSG_TO_MASTER, MPI_COMM_WORLD);
	assert(status == MPI_SUCCESS);
	LOG("Cluster %d Sent the new minimum to master\n", tsp->cluster_id);
}


partition_interval_t get_next_partition(tsp_t_pointer tsp) {	
	partition_interval_t ret;
	// if (tsp->cluster_id == 1) {
		int buffer[2];
		MPI_Status status;
		LOG("Cluster %d Will send a new request for a partition interval\n", tsp->cluster_id);
		buffer[0] = PARTITION_REQUEST;
		buffer[1] = tsp->processed_partitions;
		int return_status = MPI_Send(buffer, 2, MPI_INT, 0, MSG_TO_MASTER, MPI_COMM_WORLD);
		assert(return_status == MPI_SUCCESS);
		LOG("Cluster %d Will receive a partition interval\n", tsp->cluster_id);
		return_status = MPI_Recv(&ret, sizeof(partition_interval_t), MPI_BYTE, 0, PARTITION_RESPONSE, MPI_COMM_WORLD, &status);
		assert(return_status == MPI_SUCCESS);
		LOG("Cluster %d Received a partition interval\n", tsp->cluster_id);
	// } else {
	// 	ret.start = -1;
	// }
	return ret;
}
