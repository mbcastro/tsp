#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>

#include "types.h"
#include "tsp.h"
#include "job.h"

#define MAXX	100
#define MAXY	100
typedef struct {
	int x, y;
} coor_t;

static struct timeval _t1, _t2;
static struct timezone _tz;
static unsigned long _temps_residuel = 0;

#define top1() gettimeofday(&_t1, &_tz)
#define top2() gettimeofday(&_t2, &_tz)

void init_cpu_time(void) {
	top1(); top2();
	_temps_residuel = 1000000L * _t2.tv_sec + _t2.tv_usec - 
	(1000000L * _t1.tv_sec + _t1.tv_usec );
}

/* retourne des microsecondes */
unsigned long cpu_time(void) {
	return 1000000L * _t2.tv_sec + _t2.tv_usec - 
	(1000000L * _t1.tv_sec + _t1.tv_usec ) - _temps_residuel;
}


int    Argc;
char   **Argv;
int    InitSeed = 20;
int    minimum;

MUTEX_CREATE(mutex);

int          nb_workers;
TSPqueue     q;
DistTab_t    distance;


void genmap (int n, DTab_t pairs) {
	typedef coor_t coortab_t [MAXE];
	int tempdist [MAXE];
	coortab_t towns;
	int i, j, k, x =0;
	int dx, dy, tmp;

	srand (atoi(Argv[3]));

	for (i=0; i<n; i++) {
		towns [i].x = rand () % MAXX;
		towns [i].y = rand () % MAXY;
	}

	for (i=0; i<n; i++) {
		for (j=0; j<n; j++) {
			dx = towns [i].x - towns [j].x;
			dy = towns [i].y - towns [j].y;
			tempdist [j] = (int) sqrt ((double) ((dx * dx) + (dy * dy)));
		}

		for (j=0; j<n; j++) {
			tmp = INT_MAX;
			for (k=0; k<n; k++) {
				if (tempdist [k] < tmp) {
					tmp = tempdist [k];
					x = k;
				}
			}
			tempdist [x] = INT_MAX;
			pairs [i][j].ToCity = x;
			pairs [i][j].dist = tmp;
		}
	}
}


void InitDistance (DistTab_t *distance) {
	distance->NrTowns = atoi (Argv[2]);
	genmap (distance->NrTowns, distance->dst); 
}

void PrintDistTab () {
	int i, j;

	printf ("distance.NrTowns = %d\n", distance.NrTowns);

	for (i=0; i<distance.NrTowns; i++) {
		printf ("distance.dst [%1d]",i);
		for (j=0; j<distance.NrTowns; j++) {
			printf (" [d:%2d, to:%2d] ", distance.dst[i][j].dist, distance.dst[i][j].ToCity);
		}
		printf (";\n\n");
	}
	printf ("done ...\n");
}


int main (int argc, char **argv) {
#ifdef MT
	long i;
	void *status;

	pthread_t *tids;
	pthread_attr_t att;
#endif
	unsigned long temps;

	if (argc != 4) {
		fprintf (stderr, "Usage: %s <nb_threads > <ncities> <seed> \n",argv[0]);
		exit (1);
	}

	init_cpu_time();

	Argc = argc;
	Argv = argv;

#ifdef MT
	pthread_mutex_init (&mutex, NULL); 
#endif

	minimum = INT_MAX;
	nb_workers = atoi (argv[1]);

	printf ("nb_threads = %3d ncities = %3d\n", nb_workers, atoi(argv[2]));

	init_queue (&q);
	InitDistance (&distance);

	GenerateJobs ();

	top1 ();

#ifdef MT
	pthread_attr_init (&att);
 /* pthread_attr_setstacksize (&att, 20000); */
	pthread_attr_setscope (&att, PTHREAD_SCOPE_SYSTEM);

	tids = (pthread_t *) malloc (sizeof(pthread_t) * nb_workers);

	for (i=0; i<nb_workers; i++) {
		pthread_create (&tids[i], NULL, worker, (void *)i);
		printf ("tid %x\n", tids [i]);
	}

	for (i=0; i<nb_workers; i++) {
		pthread_join (tids[i], &status);
	}
#else
	worker ((void *)1);
#endif
	top2 ();
	temps = cpu_time();
	printf("time = %ld.%03ldms\n", temps/1000, temps%1000);
	return 0;
}

