#include <stdlib.h>
#include <string.h>
#include "tsp_mppa.h"

portal_t *create_read_portal (char *path, void* buffer, unsigned long buffer_size, int trigger, void (*function)(mppa_sigval_t)) {
	portal_t *ret = (portal_t*) malloc (sizeof (portal_t));
	int status;
	ret->file_descriptor = mppa_open(path, O_RDONLY);
	assert(ret->file_descriptor != -1);

	mppa_aiocb_t tmp[1] = {MPPA_AIOCB_INITIALIZER(ret->file_descriptor, buffer, buffer_size)};
	memcpy(&ret->portal, tmp, sizeof(mppa_aiocb_t));
	
	if (trigger) 
		mppa_aiocb_set_trigger(&ret->portal, trigger);
	
	if (function)
		mppa_aiocb_set_callback(&ret->portal, function);
	
	status = mppa_aio_read(&ret->portal);
	assert(status == 0);

	return ret;
}


portal_t *create_write_portal (char *path, int min_rank, int max_rank) {
	portal_t *ret = (portal_t*) malloc (sizeof (portal_t));
	ret->file_descriptor = mppa_open(path, O_WRONLY);
	assert(ret->file_descriptor != -1);
	// if (min_rank > 0) {
	// 	int i, status;
	// 	int size = max_rank - min_rank + 1;
	// 	int ranks[size];
	// 	for (i = 0; i < size; i++)
	// 		ranks[i] = min_rank + i;
	// 	status = mppa_ioctl(ret->file_descriptor, MPPA_TX_SET_RX_RANKS, size, ranks);
	// 	assert(status == 0);
	// }
   	return ret;
}


void close_portal(portal_t *portal) {
	mppa_close(portal->file_descriptor);
	free(portal);
}