#include <stdio.h>
#include <stdlib.h>
#include <poti.h>

#include "defs.h"


void write_header() {
	poti_header (1, 1);

	//Defining my types
	poti_DefineContainerType ("ROOT", "0", "ROOT");
	poti_DefineContainerType ("CLUSTER", "ROOT", "CLUSTER");
	poti_DefineContainerType("THREAD", "CLUSTER", "THREAD");

	poti_DefineStateType("CLUSTER_STATE", "CLUSTER", "CLUSTER_STATE");
	poti_DefineStateType("THREAD_STATE", "THREAD", "THREAD_STATE");
	
	poti_DefineEntityValue("e", "THREAD_STATE", "EXECUTING",			"0.0 1.0 0.0");
	poti_DefineEntityValue("b", "THREAD_STATE", "WAITING_BARRIER",		"1.0 0.0 0.0");

	poti_DefineEntityValue("g", "THREAD_STATE", "GENERATING_TASKS",		"1.0 1.0 0.0");
	poti_DefineEntityValue("w", "THREAD_STATE", "WAITING_FOR_TASKS",	"1.0 0.2 0.0");
	poti_DefineEntityValue("p", "THREAD_STATE", "PROCESSING_TASKS",		"0.0 0.0 1.0");
}

void create_containers(int nb_clusters, int nb_threads) {
	int cluster, thread;
	
	poti_CreateContainer (0, "root", "ROOT", "0", "root");  
	
	for (cluster = 0; cluster < nb_clusters; cluster++) {

		char cluster_id[256];
		char cluster_id2[256];
		char cluster_desc[256];
		sprintf(cluster_id, "Cluster_%03d", cluster);
		sprintf(cluster_id2, "Cluster_%03d_Aux", cluster);
		sprintf(cluster_desc, "Cluster %d", cluster);
		poti_CreateContainer (0, cluster_id, "CLUSTER", "root", cluster_desc);
		poti_CreateContainer (0, cluster_id2, "THREAD", cluster_id, "Cluster");
		for (thread = 0; thread < nb_threads; ++thread) {
			char thread_id[256];
			char thread_desc[256];
			sprintf(thread_id, "Thread_%03d_%03d", cluster, thread);
			sprintf(thread_desc, "Thread %d", thread);
			poti_CreateContainer (0, thread_id, "THREAD", cluster_id, thread_desc);
		}    

	}
}

void destroy_containers(double when, int nb_clusters, int nb_threads) {
	int cluster, thread;

	for (cluster = 0; cluster < nb_clusters; cluster++) {
		
		for (thread = 0; thread < nb_threads; ++thread) {
			char thread_id[256];
			sprintf(thread_id, "Thread_%03d_%03d", cluster, thread);    
			poti_DestroyContainer (when,"THREAD", thread_id);
		}    

		char cluster_id[256];
		sprintf(cluster_id, "Cluster_%03d", cluster);
		poti_DestroyContainer (when, "CLUSTER", cluster_id);
		sprintf(cluster_id, "Cluster_%03d_Aux", cluster);
		poti_DestroyContainer (when, "CLUSTER", cluster_id);
	}
	poti_DestroyContainer (when, "ROOT", "root");
}



int main (int argc, char **argv) {
	
	FILE *fp;
	
	write_header();
	create_containers (16, 16);



	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Couldn't open %s for reading.\n", argv[1]);
		exit(0);
	}

	unsigned long when, start, end;
	int cluster_id, thread_id, state;

	start = -1;
	end = -1;

	while (fscanf(fp, "%lu\t%d\t%d\t%d\n", &when, &cluster_id, &thread_id, &state) == 4) {
		if (start == -1) start = when;
		double new_when = 1.0 * (when - start) / 1000000;
		char thread[256];
		sprintf(thread, "Thread_%03d_%03d", cluster_id, thread_id);
		char cluster[256];
		sprintf(cluster, "Cluster_%03d_Aux", cluster_id);

		switch (state) {
			case GENERATING_TASKS:
				poti_SetState (new_when, thread, "THREAD_STATE", "g");
				end = new_when;
				break;
			case WAITING_FOR_TASKS:
				poti_SetState (new_when, thread, "THREAD_STATE", "w");
				end = new_when;
				break;
			case PROCESSING_TASKS:
				poti_SetState (new_when, thread, "THREAD_STATE", "p");
				end = new_when;
				break;
			case WAITING_FIRST_BARRIER:
			case WAITING_SECOND_BARRIER:
				poti_SetState (new_when, cluster, "THREAD_STATE", "b");
				end = new_when;
				break;
			case RELEASED_FIRST_BARRIER:
			case RELEASED_SECOND_BARRIER:
				poti_SetState (new_when, cluster, "THREAD_STATE", "e");
				end = new_when;
				break;
			case STARTING_EXECUTION:
				end = new_when;
				break;
			case ENDING_EXECUTION:
				end = new_when;
				break;
			default:
				break;
		}
	}
	
	fclose(fp);

	destroy_containers(end, 16, 16);

return 0;
}