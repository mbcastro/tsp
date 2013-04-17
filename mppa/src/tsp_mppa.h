#ifndef __TSP_MPPA_H
#define __TSP_MPPA_H

#include "defs.h"
#include <mppaipc.h>

#define WORKERS_PER_CLUSTER 16

//to
#define MINIMUM_SLAVE_TO_MASTER "/mppa/portal/128:42/"
#define MINIMUM_MASTER_TO_SLAVES "/mppa/portal/[0..15]:43/"

typedef struct  {
	int file_descriptor;
	mppa_aiocb_t portal;
} portal_t;

portal_t *create_read_portal (char *path, void* buffer, unsigned long buffer_size, int trigger, void (*function)(mppa_sigval_t));
portal_t *create_write_portal (char *path, int min_rank, int max_rank);
void close_portal(portal_t *portal);

#endif // __TSP_MPPA_H