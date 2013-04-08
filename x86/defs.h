#define MAXE	30

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

typedef struct {
	int ToCity ;
	int dist ;
} pair_t ;

typedef pair_t DistArray_t [MAXE] ;
typedef DistArray_t DTab_t [MAXE] ;

typedef struct {
	int NrTowns ;
	DTab_t dst ;
} DistTab_t ;

/* Job types */

typedef int Path_t [MAXE] ;

typedef struct {
	int len ;
	Path_t path ;
} Job_t ;

/* TSQ Queue */

typedef struct Maillon {
	Job_t tsp_job ;
	struct Maillon *next ;
} Maillon ;

typedef struct {
	Maillon *first ;
	Maillon *last ;
	int end;
	MUTEX_CREATE(mutex);
} TSPqueue ;




