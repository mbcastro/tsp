#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cpu_affinity.h"

static const char *nexttoken(const char *q,  int sep) {
	if (q)
		q = strchr(q, sep);
	if (q)
		q++;
	return q;
}


static int cpulist_parse(const char *str, cpu_set_t *set, size_t setsize) {
	const char *p, *q;
	q = str;

	CPU_ZERO_S(setsize, set);

	while (p = q, q = nexttoken(q, ','), p) {
		unsigned int a;	/* beginning of range */
		unsigned int b;	/* end of range */
		unsigned int s;	/* stride */
		const char *c1, *c2;

		if (sscanf(p, "%u", &a) < 1)
			return 1;
		b = a;
		s = 1;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');
		if (c1 != NULL && (c2 == NULL || c1 < c2)) {
			if (sscanf(c1, "%u", &b) < 1)
				return 1;
			c1 = nexttoken(c1, ':');
			if (c1 != NULL && (c2 == NULL || c1 < c2))
				if (sscanf(c1, "%u", &s) < 1) {
					return 1;
			}
		}

		if (!(a <= b))
			return 1;
		while (a <= b) {
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	return 0;
}


/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9").
 */
static char *cpulist_create(char *str, size_t len, cpu_set_t *set, size_t setsize) {
	int i;
	char *ptr = str;
	int entry_made = 0;
	size_t max = cpuset_nbits(setsize);

	for (i = 0; i < max; i++) {
		if (CPU_ISSET_S(i, setsize, set)) {
			int j, rlen;
			int run = 0;
			entry_made = 1;
			for (j = i + 1; j < max; j++) {
				if (CPU_ISSET_S(j, setsize, set))
					run++;
				else
					break;
			}
			if (!run)
				rlen = snprintf(ptr, len, "%d,", i);
			else if (run == 1) {
				rlen = snprintf(ptr, len, "%d,%d,", i, i + 1);
				i++;
			} else {
				rlen = snprintf(ptr, len, "%d-%d,", i, i + run);
				i += run;
			}
			if (rlen < 0 || rlen + 1 > len)
				return NULL;
			ptr += rlen;
			len -= rlen;
		}
	}
	ptr -= entry_made;
	*ptr = '\0';

	return str;
}


cpu_set_t* mask_for_partition(int partition, char** machine) {
	cpu_set_t *mask = (cpu_set_t *)malloc(sizeof(cpu_set_t));
	char *list = machine[partition];
	int status = cpulist_parse(list, mask, sizeof(cpu_set_t));
	assert(status == 0);
	char buffer[256];
	cpulist_create(buffer, 256, mask,  sizeof(cpu_set_t));
	LOG("CPU Mask for partition %d: %s\n", partition, buffer);
	return mask;
}