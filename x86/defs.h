#ifndef __DEFS_H
#define __DEFS_H

#define MAX_TOWNS	30
#define MAX_HOPS	5
#define MAX_GRID_X	100
#define MAX_GRID_Y	100

#ifdef MT 

#include <pthread.h>

#ifdef CAS

#define MUTEX_CREATE(m)	int m
#define MUTEX_INIT(m) 	m = 0
#define MUTEX_LOCK(m) 	while(!__sync_bool_compare_and_swap(&m, 0, 1))
#define MUTEX_UNLOCK(m) while(!__sync_bool_compare_and_swap(&m, 1, 0))

#else

#define MUTEX_CREATE(m)	pthread_mutex_t m
#define MUTEX_INIT(m) 	pthread_mutex_init(&m, NULL)
#define MUTEX_LOCK(m) 	pthread_mutex_lock(&m)
#define MUTEX_UNLOCK(m) pthread_mutex_unlock(&m)

#endif //CAS

#else

#define MUTEX_CREATE(m)	/* none */
#define MUTEX_INIT(m)	/* none */
#define MUTEX_LOCK(m)			/* none */
#define MUTEX_UNLOCK(m)		/* none */

#endif //MT

#ifdef DEBUG
#define LOG printf
#else
#define LOG(...) 
#endif //DEBUG


#ifdef DEF_RAND
#define RAND_INIT(s) srand(s)
#define RAND_NEXT() rand()
#else
void simple_rng_initialize(int seed);
inline unsigned int simple_rng_next();
#define RAND_INIT(s) simple_rng_initialize(s)
#define RAND_NEXT() simple_rng_next()
#endif //DEF_RAND

#endif //__DEFS_H
	