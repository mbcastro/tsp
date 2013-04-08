#ifndef __DEFS_H
#define __DEFS_H

#define MAX_TOWNS	30
#define MAX_HOPS	3
#define MAX_GRID_X	100
#define MAX_GRID_Y	100

#ifdef MT 

#include <pthread.h>
#define MUTEX_CREATE(m)	pthread_mutex_t m
#define MUTEX_INIT(m) 	pthread_mutex_init(&m, NULL)
#define MUTEX_LOCK(m) 	pthread_mutex_lock(&m)
#define MUTEX_UNLOCK(m) pthread_mutex_unlock(&m)

#else

#define MUTEX_CREATE(m)	/* none */
#define MUTEX_INIT(m)	/* none */
#define LOCK(m)			/* none */
#define UNLOCK(m)		/* none */

#endif

#ifdef DEBUG
#define LOG printf
#else
#define LOG(...) 
#endif

#endif