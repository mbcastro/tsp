#include <mppa/osconfig.h>

#include "tsp_mppa.h"

static int clusters;
MUTEX_CREATE(min_slave_to_master_lock, static);

void callback_slave (mppa_sigval_t sigval);

static int *comm_buffer;
static int comm_buffer_size;
static broadcast_t *broad;

static int min_local = INT_MAX;

static tsp_t_pointer tsp_instance;

int main(int argc, char **argv) {	
	int i, n_threads = atoi(argv[0]);
	clusters = atoi(argv[3]);
	int rank = __k1_get_cluster_id();
	LOG("Cluster %d/%d starting.\n", rank, clusters);

	MUTEX_INIT(min_slave_to_master_lock);
	
	comm_buffer_size = (clusters + 1) * sizeof(int);
	comm_buffer = (int *) malloc(comm_buffer_size);
	for (i = 0; i <= clusters; i++)
		comm_buffer[i] = INT_MAX;

	barrier_t *sync_barrier = create_slave_barrier (BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE);
	broad = create_broadcast (clusters, BROADCAST_MASK, comm_buffer, comm_buffer_size, TRUE, callback_slave);

	barrier_wait(sync_barrier);

	tsp_instance = init_execution(rank, clusters, n_threads, atoi(argv[1]), atoi(argv[2]));
	start_execution(tsp_instance);
	end_execution(tsp_instance);

    LOG("Cluster %d exiting. Min: %d\n", rank, min_local);
    
    barrier_wait(sync_barrier);

	close_broadcast(broad);
	close_barrier(sync_barrier);
	free(comm_buffer);

	mppa_exit(0);
	return 0;
}


void new_minimun_distance_found(tsp_t_pointer tsp_instance) {
	MUTEX_LOCK(min_slave_to_master_lock);
	min_local = tsp_get_shortest_path(tsp_instance);
	broadcast (broad, &min_local, sizeof(int));	
	MUTEX_UNLOCK(min_slave_to_master_lock);
}

void callback_slave (mppa_sigval_t sigval) {	
	int i, min = INT_MAX;
	for (i = 0; i < clusters; i++)
		if (comm_buffer[i] < min) 
			min = comm_buffer[i];
	tsp_update_minimum_distance(tsp_instance, min);	
	LOG("Slave: Received a callback. %d\n", min);
}