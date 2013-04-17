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
static int rank, clusters;

int main(int argc, char **argv) {	
	int status;
	execution_info_t exec_info;

	clusters = atoi(argv[3]);
	rank = __k1_get_cluster_id();
	LOG("Cluster %d/%d starting.\n", rank, clusters);

	portal_t *results_portal = create_write_portal (RESULTS_GATHERING);
	min_slave_to_master_portal = create_write_portal (MINIMUM_SLAVE_TO_MASTER);
	
	exec_info = start_execution(rank, clusters, atoi(argv[0]), atoi(argv[1]), atoi(argv[2]));
	
	status = mppa_pwrite(results_portal->file_descriptor, &exec_info, sizeof(execution_info_t), rank * sizeof(execution_info_t));
   	assert(status > 0);
    
    close_portal(results_portal);
    close_portal(min_slave_to_master_portal);	

	mppa_exit(0);
	return 0;
}

void new_minimun_distance_found(int num_worker, int length) {
	int status = mppa_pwrite(min_slave_to_master_portal->file_descriptor, &length, sizeof(int), rank * clusters + num_worker * sizeof(int));
   	assert(status > 0);
}