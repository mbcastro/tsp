#include "defs.h"

const unsigned long long FACTORIAL_TABLE[] = {
/*00*/ 0x1,
/*01*/ 0x1,
/*02*/ 0x2,
/*03*/ 0x6,
/*04*/ 0x18,
/*05*/ 0x78,
/*06*/ 0x2d0,
/*07*/ 0x13b0,
/*08*/ 0x9d80,
/*09*/ 0x58980,
/*10*/ 0x375f00,
/*11*/ 0x2611500,
/*12*/ 0x1c8cfc00,
/*13*/ 0x17328cc00,
/*14*/ 0x144c3b2800,
/*15*/ 0x13077775800,
/*16*/ 0x130777758000,
/*17*/ 0x1437eeecd8000,
/*18*/ 0x16beecca730000,
/*19*/ 0x1b02b9306890000,
/*20*/ 0x21c3677c82b40000
};


const static unsigned int simple_rng_s_mw_default = 521288629;
const static unsigned int simple_rng_s_mz_default = 362436069;

struct rand_state_struct simple_rng_initialize(int seed) {
	struct rand_state_struct ret;
	unsigned int n1, n2;
	n1 = (seed * 104623) % 4294967296;
	ret.w = (n1) ? n1 : simple_rng_s_mw_default;
	n2 = (seed * 48947) % 4294967296;
	ret.z = (n2) ? n2 : simple_rng_s_mz_default;
	return ret;
}


/// SimpleRNG is a simple random number generator based on
/// George Marsaglia's MWC (multiply with carry) generator.
/// Although it is very simple, it passes Marsaglia's DIEHARD
/// series of random number generator tests.
/// Written by John D. Cook

inline unsigned int simple_rng_next(struct rand_state_struct *current_state) {
	// 0 <= u < 2^32
	unsigned int u;
	current_state->z = 36969 * (current_state->z & 65535) + (current_state->z >> 16); 
	current_state->w = 18000 * (current_state->w & 65535) + (current_state->w >> 16);
	u = (current_state->z << 16) + current_state->w;
	return u;
}


static const char *nexttoken(const char *q,  int sep) {
	if (q)
		q = strchr(q, sep);
	if (q)
		q++;
	return q;
}

int* par_parse(const char *str) {
	const char *p, *q;
	q = str;
	
	int *set, size, max_size;
	max_size = 1024;
	size = 0;
	set = (int*)calloc(max_size, sizeof(int));

	while (p = q, q = nexttoken(q, ','), p) {
		unsigned int a;	/* beginning of range */
		unsigned int b;	/* end of range */
		unsigned int s;	/* stride */
		const char *c1, *c2;

		if (sscanf(p, "%u", &a) < 1) {
			free(set);
			return NULL;
		}
		b = a;
		s = 1;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');
		if (c1 != NULL && (c2 == NULL || c1 < c2)) {
			if (sscanf(c1, "%u", &b) < 1) {
				free(set);
				return NULL;
			}
			c1 = nexttoken(c1, ':');
			if (c1 != NULL && (c2 == NULL || c1 < c2))
				if (sscanf(c1, "%u", &s) < 1) {
					free(set);
					return NULL;
				}
		}

		if (!(a <= b)) {
			free(set);
			return NULL;
		}

		while (a <= b) {
			set[size++] = a;
			a += s;
		}
	}

	return set;
}
