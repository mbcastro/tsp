#ifndef __TSP_MPPA_H
#define __TSP_MPPA_H

#include "defs.h"

#define WORKERS_PER_CLUSTER 16

//to/from
#define WORK_DISTRIBUTION "/mppa/channel/5:18/128:42"
#define RESULTS_GATHERING "/mppa/channel/128:40/5:10"

#endif // __TSP_MPPA_H