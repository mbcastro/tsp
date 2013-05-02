#ifndef __COMMON_MAIN_H
#define __COMMON_MAIN_H

#include "exec.h"
#include "timer.h"

struct main_pars {
	int *nb_threads;
	int *nb_towns;
	int seed;
	int *nb_clusters;
	int nb_executions;	
	char *machine;
};

struct main_pars init_main_pars (int argc, char **argv);
void run_main (struct main_pars pars);

extern void run_tsp (int nb_threads, int nb_towns, int seed, int nb_clusters, char* machine);
extern void wait_barrier (int limit);
pthread_t *spawn (tsp_t_pointer *tsp, int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed, char* machine);

#endif //__COMMON_MAIN_H