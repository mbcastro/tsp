#define _GNU_SOURCE             

#include "common_main.h"
#include "cpu_affinity.h"

struct execution_parameters {
	int cluster;
	int nb_clusters;
	int nb_partitions;
	int nb_threads;
	int nb_towns;
	int seed;
	tsp_t_pointer *tsp;
};

static void *spawn_worker(void* params) {
	struct execution_parameters *p = (struct execution_parameters *)params;
	(*p->tsp) = init_execution(p->cluster, p->nb_clusters, p->nb_partitions, p->nb_threads, p->nb_towns, p->seed);
	
	wait_barrier (p->nb_clusters);
	start_execution(*p->tsp);
	wait_barrier (p->nb_clusters);
	
	end_execution(*p->tsp);

	free(params);
	return NULL;
}

pthread_t *spawn (tsp_t_pointer *tsp, int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed, char* machine) {
	pthread_t *tid = (pthread_t *)malloc (sizeof(pthread_t));
	struct execution_parameters *params = (struct execution_parameters*) malloc (sizeof(struct execution_parameters));
	params->cluster = cluster_id;
	params->nb_clusters = nb_clusters;
	params->nb_partitions = nb_partitions;
	params->nb_threads = nb_threads;
	params->nb_towns = nb_towns;
	params->seed = seed;
	params->tsp = tsp;

	int status = pthread_create (tid, NULL, spawn_worker, params);
	assert (status == 0);

	if (machine) {
		char **machine_sched = get_machine_sched(machine);
		cpu_set_t *cpu_set = mask_for_partition(cluster_id, machine_sched);
		status = pthread_setaffinity_np (*tid, sizeof(cpu_set_t), cpu_set);
		assert (status == 0);
		free(cpu_set);
	}

	return tid;	
}


struct main_pars init_main_pars (int argc, char **argv) {
	struct main_pars ret;
	
	CHECK_PAGE_SIZE();
	
	if (argc != 5 && argc != 6 && argc != 7) {
		fprintf (stderr, "Usage: %s <nb_threads> <nb_towns> <seed> <nb_clusters> [nb_executions machine]\n", argv[0]);
		exit(1);
	}

	ret.nb_threads = par_parse (argv[1]);
	assert(ret.nb_threads);
	ret.nb_towns = par_parse (argv[2]);
	assert(ret.nb_towns);
	ret.seed = atoi(argv[3]);
	ret.nb_clusters = par_parse (argv[4]);
	assert(ret.nb_clusters);
	ret.nb_executions = (argc == 6) ? atoi(argv[5]) : 1;
	assert(ret.nb_executions > 0);
	ret.machine = (argc == 7) ? argv[6] : NULL;

	init_time();

	return ret;
}

void run_main (struct main_pars pars) {
	int execution, town, cluster, thread;
	town = 0;
	while (pars.nb_towns[town] != 0) {
		cluster = 0;
		while (pars.nb_clusters[cluster] != 0) {
			thread = 0;
			while (pars.nb_threads[thread] != 0) {
				assert (pars.nb_threads[thread] > 0);
				assert (pars.nb_towns[town] <= MAX_TOWNS);
				assert (pars.nb_clusters[cluster] > 0);
				for (execution = 0; execution < pars.nb_executions; execution++)
					run_tsp(pars.nb_threads[thread], pars.nb_towns[town], pars.seed, pars.nb_clusters[cluster], pars.machine);
				thread++;
			}
			cluster++;
		}
		town++;
	}

}