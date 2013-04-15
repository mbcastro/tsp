#include <fcntl.h>
#include <mppaipc.h>
#define CONFIGURE_AMP_MAIN_STACK_SIZE 200000
#include <mppa/osconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tsp_mppa.h"
#include "exec.h"

int main(int argc, char **argv) {	
	int ret = 0;
	int pars[3], res, status;

	int pars_channel = mppa_open(WORK_DISTRIBUTION, O_RDONLY);
  	assert(pars_channel != -1);
	int res_channel = mppa_open(RESULTS_GATHERING, O_WRONLY);
	assert(res_channel != -1);

	status = mppa_read(pars_channel, pars, 3 * sizeof(int));
	assert(status == (3 * sizeof(int))); 

	res = start_execution(pars[0], pars[1], pars[2]);
	
	status = mppa_write(res_channel, &res, sizeof(int));
   	assert(status == sizeof(int));

	mppa_close(pars_channel);
   	mppa_close(res_channel);
		
	mppa_exit(ret);
	return 0;
}