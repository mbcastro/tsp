#ifndef __CPU_AFFINITYY_H
#define __CPU_AFFINITYY_H

#include <sched.h>

#define cpuset_nbits(setsize)	 (8 * (setsize))

extern const char *IDCHIRE[];
extern const char *IDROUILLE[];

char **get_machine_sched(char *machine_name);
cpu_set_t* mask_for_partition(int partition, char** machine);

#endif

