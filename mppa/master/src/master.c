#include <stdio.h>
#include <stdlib.h>
#include <mppa/osconfig.h>
#include <mppaipc.h>
#include <assert.h>
#include "tsp_mppa.h"

int main (int argc, const char **argv) {
	int status = 0;
	int rank = 5;
	int output_channel, input_channel, pid, res, ret;

	int pars[3];
	pars[0] = atoi(argv[1]);
	pars[1] = atoi(argv[2]);
	pars[2] = atoi(argv[3]);
	

	printf("Starting execution. Towns = %d, Seed = %d\n", pars[0], pars[1]);

 	output_channel = mppa_open(WORK_DISTRIBUTION, O_WRONLY);
  	assert(output_channel != -1);
  	input_channel = mppa_open(RESULTS_GATHERING, O_RDONLY);
  	assert(input_channel != -1);


	pid = mppa_spawn(rank, 0, "tsp_lock_mppa_slave", argv + 2, 0);
	assert(pid >= 0);
	
	status = mppa_write(output_channel, pars, sizeof(pars));
  	assert(status == 3 * sizeof(int));

  	status = mppa_read(input_channel, &res, sizeof(int));
    assert(status == (sizeof(int))); 

    printf("Res %d\n", res);
	
	//start_execution(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	mppa_close(output_channel);
    mppa_close(input_channel);

	status = 0;
   	if ((ret = mppa_waitpid(rank, &status, 0)) < 0) {
    	printf("[I/O] Waitpid on cluster 5 failed.\n");
     	mppa_exit(ret);
   	}  

	mppa_exit(0);
	return 0;
}
