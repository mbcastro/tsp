#ifndef __EXEC_H
#define __EXEC_H

#include "defs.h"
#include "tsp.h"

typedef struct  {
	unsigned int partition;
	unsigned int shortest_path_length;
} execution_info_t;

tsp_t_pointer init_execution(int partition, int n_partitions, int n_workers, int n_towns, int seed);
execution_info_t start_execution(tsp_t_pointer tsp);
void end_execution (tsp_t_pointer tsp);

void print_execution_info(execution_info_t exec_info);
//minimum hint
void update_minimum(tsp_t *tsp, int new_minimum);


#endif //__EXEC_H