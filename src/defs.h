#ifndef __DEFS_H
#define __DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
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

#define GET_3TH_ARG(arg1, arg2, arg3, ...) arg3

//COND_VAR_CREATE
#define COND_VAR_CREATE_1(cv) struct cond_var_struct cv
#define COND_VAR_CREATE_2(cv, modifiers) modifiers COND_VAR_CREATE_1(cv)
#define COND_VAR_CREATE_MACRO_CHOOSER(...) \
    GET_3TH_ARG(__VA_ARGS__, COND_VAR_CREATE_2, COND_VAR_CREATE_1, )
#define COND_VAR_CREATE(...) COND_VAR_CREATE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define COND_VAR_INIT(cv) pthread_mutex_init(&cv.mutex, NULL); pthread_cond_init (&cv.cond_var, NULL)
#define COND_VAR_SIGNAL(cv) pthread_cond_signal(&cv.cond_var)
#define COND_VAR_BROADCAST(cv) pthread_cond_broadcast(&cv.cond_var)
#define COND_VAR_WAIT(cv) pthread_cond_wait (&cv.cond_var, &cv.mutex)
#define COND_VAR_MUTEX_LOCK(cv) pthread_mutex_lock(&cv.mutex)
#define COND_VAR_MUTEX_UNLOCK(cv) pthread_mutex_unlock(&cv.mutex)

#ifdef CAS

#define MUTEX_CREATE_1(m)	unsigned int m
#define MUTEX_INIT(m) 		m = 0
#define MUTEX_LOCK(m) 		while(__sync_lock_test_and_set(&m, 1))
#define MUTEX_UNLOCK(m) 	__sync_lock_release(&m)	

#else

#define MUTEX_CREATE_1(m)	pthread_mutex_t m
#define MUTEX_INIT(m) 		pthread_mutex_init(&m, NULL)
#define MUTEX_LOCK(m) 		pthread_mutex_lock(&m)
#define MUTEX_UNLOCK(m) 	pthread_mutex_unlock(&m)

#endif //CAS

#define MUTEX_CREATE_2(m, modifiers) modifiers MUTEX_CREATE_1(m)
#define MUTEX_CREATE_MACRO_CHOOSER(...) \
    GET_3TH_ARG(__VA_ARGS__, MUTEX_CREATE_2, MUTEX_CREATE_1, )
#define MUTEX_CREATE(...) MUTEX_CREATE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#else //end MT

//Sequential version
#define MUTEX_CREATE(...)	/* none */
#define MUTEX_INIT(m)		/* none */
#define MUTEX_LOCK(m)		/* none */
#define MUTEX_UNLOCK(m)		/* none */

#define COND_VAR_CREATE(...) 
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

#define RAND_STATE_T int 
#define RAND_INIT(s) srand(s); 0
#define RAND_NEXT(state) rand()

#else //Our random thread-safe impl.

struct rand_state_struct {
	unsigned int w;
	unsigned int z;
};

#define RAND_STATE_T struct rand_state_struct

struct rand_state_struct simple_rng_initialize(int seed);
inline unsigned int simple_rng_next(struct rand_state_struct *current_state);

#define RAND_INIT(s) simple_rng_initialize(s)
#define RAND_NEXT(state) simple_rng_next(&state)

#endif //DEF_RAND

int* par_parse(const char *str);

#endif //__DEFS_H
