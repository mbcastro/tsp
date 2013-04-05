#include <stdio.h>
#include "types.h"
#include "job.h"

#define MAXHOPS		3

extern int          minimum;
extern TSPqueue     q;
extern DistTab_t    distance;
MUTEX_CREATE(mutex);

int present (int city, int hops, Path_t path) {
	unsigned int i;

	for (i=0; i<hops; i++)
		if (path [i] == city) return 1;
	return 0;
}

void tsp (int hops, int len, Path_t path, int *cuts, int num_worker) {
	int i;
	int city, me, dist;

	if (len >= minimum) {
		(*cuts)++;
		return;
	}

	if (hops == distance.NrTowns) {
		MUTEX_LOCK(mutex);
		if (len < minimum) {
			minimum = len;
			printf ("worker[%d] finds path len = %3d :", num_worker, len);
			for (i=0; i < distance.NrTowns; i++)
				printf ("%2d ", path[i]);
			printf ("\n");
		}
		MUTEX_UNLOCK(mutex);
	} else {
		me = path [hops-1];

		for (i=0; i < distance.NrTowns; i++) {
			city = distance.dst [me][i].ToCity;
			if (!present (city, hops, path)) {
				path [hops] = city;
				dist = distance.dst[me][i].dist;
				tsp (hops+1, len+dist, path, cuts, num_worker);
			}
		}
	}
}

void distributor (int hops, int len, Path_t path, TSPqueue *q, DistTab_t distance) {
	Job_t j;
	int i;
	int me, city, dist;

	if (hops == MAXHOPS) {
		j.len = len;
		for (i=0; i<hops;i++)
			j.path[i] = path[i];
		add_job (q, j);
	} else {
		me = path [hops-1];
		for (i=0;i<distance.NrTowns;i++) {
			city = distance.dst[me][i].ToCity;
			if (!present(city,hops, path)) {
				path [hops] = city;
				dist = distance.dst[me][i].dist;
				distributor (hops+1, len+dist, path, q, distance);       
			}
		}
	} 
}

void GenerateJobs () {
	Path_t path;

	path [0] = 0;
	distributor (1, 0, path, &q, distance); 
	no_more_jobs (&q);
}

void *worker (int num_worker) {
	int jobcount = 0;
	Job_t job;
	int cuts = 0;

	printf ("Worker [%2d] starts \n", num_worker);

	while (get_job (&q, &job)) {
		jobcount++;
		tsp (MAXHOPS, job.len, job.path, &cuts, num_worker);
	}

	printf ("Worker [%2d] terminates, %4d jobs done with %4d cuts.\n", num_worker, jobcount, cuts);
	return (void *) 0;
}




