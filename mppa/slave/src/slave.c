#include <fcntl.h>
#include <mppaipc.h>
#define CONFIGURE_AMP_MAIN_STACK_SIZE 200000
#include <mppa/osconfig.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {	
	int ret = 0;
		
	mppa_exit(ret);
	return 0;
}