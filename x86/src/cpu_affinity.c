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


cpu_set_t* mask_for_partition(int partition, char** machine) {
	cpu_set_t *mask = (cpu_set_t *)malloc(sizeof(cpu_set_t));
	char *list = machine[partition];
	int status = cpulist_parse(list, mask, sizeof(cpu_set_t));
	assert(status == 0);
	return mask;
}