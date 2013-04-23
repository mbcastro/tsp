#ifndef __EXEC_H
#define __EXEC_H

#include "defs.h"
#include "tsp.h"

tsp_t_pointer init_execution(int partition, int n_partitions, int n_workers, int n_towns, int seed);
void start_execution(tsp_t_pointer tsp);
void end_execution (tsp_t_pointer tsp);

//minimum hint
void update_minimum(tsp_t *tsp, int new_minimum);


#endif //__EXEC_H