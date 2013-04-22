#ifndef __EXEC_H
#define __EXEC_H

#include "defs.h"

typedef struct  {
	unsigned int partition;
	unsigned int shortest_path_length;
} execution_info_t;


execution_info_t start_execution(int partition, int n_partitions, int n_workers, int n_towns, int seed);
void print_execution_info(execution_info_t exec_info);



#endif //__EXEC_H