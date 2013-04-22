#ifndef __TSP_MPPA_H
#define __TSP_MPPA_H

#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mppaipc.h>

//to
#define BROADCAST_MASK "/mppa/portal/%d:%d"

#define BARRIER_SYNC_MASTER "/mppa/sync/128:20"
#define BARRIER_SYNC_SLAVE "/mppa/sync/[0..15]:20"

#define TRUE 1
#define FALSE 0

#define IO_NODE_RANK 128

typedef enum {
	BARRIER_MASTER,
	BARRIER_SLAVE
} barrier_mode_t;

typedef struct  {
	int file_descriptor;
	mppa_aiocb_t portal;
} portal_t;

typedef struct {
	int sync_fd_master;
	int sync_fd_slave;
	long long match;
	barrier_mode_t mode; 
} barrier_t;

typedef struct {
	int clusters;
	int includes_ionode;
	portal_t **portals;
} broadcast_t;

void set_path_name(char *path, char *template_path, int rx, int tag);

portal_t *create_read_portal (char *path, void* buffer, unsigned long buffer_size, int trigger, void (*function)(mppa_sigval_t));
portal_t *create_write_portal (char *path, int min_rank, int max_rank, int includes_ionode);
void write_portal (portal_t *portal, void *buffer, int buffer_size, int offset);
void close_portal (portal_t *portal);

broadcast_t *create_broadcast (int clusters, char *path_name, void *buffer, int buffer_size, int includes_ionode, void (*callback_function)(mppa_sigval_t));
void broadcast (broadcast_t *broadcast, void * value, int size);
void close_broadcast (broadcast_t *broadcast);

barrier_t *create_master_barrier (char *path_master, char *path_slave, int clusters);
barrier_t *create_slave_barrier (char *path_master, char *path_slave);
void barrier_wait (barrier_t *barrier);
void close_barrier (barrier_t *barrier);

#endif // __TSP_MPPA_H