#ifndef __DEFS_H
#define __DEFS_H

#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#define MAX_TOWNS		20
#define MIN_JOBS_THREAD 100
#define MAX_GRID_X		100
#define MAX_GRID_Y		100

#define CHECK_PAGE_SIZE() if (PAGE_SIZE != sysconf(_SC_PAGESIZE)) printf("WARNING: Page size is different from the compilation parameter\n")

#ifdef MT 

struct cond_var_struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond_var;
};

#define COND_VAR_CREATE(cv) struct cond_var_struct cv
#define COND_VAR_INIT(cv) pthread_mutex_init(&cv.mutex, NULL); pthread_cond_init (&cv.cond_var, NULL)
#define COND_VAR_SIGNAL(cv) pthread_cond_signal(&cv.cond_var)
#define COND_VAR_BROADCAST(cv) pthread_cond_broadcast(&cv.cond_var)
#define COND_VAR_WAIT(cv) pthread_cond_wait (&cv.cond_var, &cv.mutex)
#define COND_VAR_MUTEX_LOCK(cv) pthread_mutex_lock(&cv.mutex)
#define COND_VAR_MUTEX_UNLOCK(cv) pthread_mutex_unlock(&cv.mutex)

#ifdef CAS

#define MUTEX_CREATE(m)	unsigned int m
#define MUTEX_INIT(m) 	m = 0
#define MUTEX_LOCK(m) 	while(__sync_lock_test_and_set(&m, 1))
#define MUTEX_UNLOCK(m) __sync_lock_release(&m)	

#else

#define MUTEX_CREATE(m)	pthread_mutex_t m
#define MUTEX_INIT(m) 	pthread_mutex_init(&m, NULL)
#define MUTEX_LOCK(m) 	pthread_mutex_lock(&m)
#define MUTEX_UNLOCK(m) pthread_mutex_unlock(&m)

#endif //CAS

#else

//Sequential version
#define MUTEX_CREATE(m)	/* none */
#define MUTEX_INIT(m)	/* none */
#define MUTEX_LOCK(m)	/* none */
#define MUTEX_UNLOCK(m)	/* none */

#define COND_VAR_CREATE(cv) 
#define COND_VAR_INIT(cv) 
#define COND_VAR_SIGNAL(cv) 
#define COND_VAR_BROADCAST(cv) 
#define COND_VAR_WAIT(cv) 
#define COND_VAR_MUTEX_LOCK(cv)
#define COND_VAR_MUTEX_UNLOCK(cv) 

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
