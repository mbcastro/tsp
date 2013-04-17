#include <fcntl.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tsp_mppa.h"
#include "exec.h"
#include "timer.h"

static portal_t * min_slave_to_master_portal;
static int n_threads, rank, clusters;
static int min_from_master;

void callback_master_to_slave (mppa_sigval_t sigval) {	
	update_minimum(min_from_master);
	printf("Slave: Recebi um callback. %d\n", min_from_master);
}

int main(int argc, char **argv) {	
	n_threads = atoi(argv[0]);
	clusters = atoi(argv[3]);
	rank = __k1_get_cluster_id();
	LOG("Cluster %d/%d starting.\n", rank, clusters);

	min_slave_to_master_portal = create_write_portal (MINIMUM_SLAVE_TO_MASTER, -1, -1);
	portal_t *min_master_to_slave_portal = create_read_portal (MINIMUM_MASTER_TO_SLAVES, &min_from_master, sizeof(int), 0, callback_master_to_slave);

	start_execution(rank, clusters, n_threads, atoi(argv[1]), atoi(argv[2]));
	    
    close_portal(min_slave_to_master_portal);	
    close_portal(min_master_to_slave_portal);

    LOG("Cluster %d saindo\n", rank);

	mppa_exit(0);
	return 0;
}

void new_minimun_distance_found(int num_worker, int length) {
	printf("Vai dar pwrite %d\n", num_worker);
	int status = mppa_pwrite(min_slave_to_master_portal->file_descriptor, &length, sizeof(int), n_threads * sizeof(int) * rank + num_worker * sizeof(int));
   	assert(status == sizeof(int));
}