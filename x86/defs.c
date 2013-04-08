#include "defs.h"

static unsigned int simple_rng_s_mw = 521288629;
static unsigned int simple_rng_s_mz = 362436069;

void simple_rng_initialize(int seed) {
	unsigned int n1, n2;
	n1 = (seed * 104623) % 4294967296;
	if (n1) simple_rng_s_mw = n1;
	n2 = (seed * 48947) % 4294967296;
	if (n2) simple_rng_s_mz = n2;
}


/// SimpleRNG is a simple random number generator based on
/// George Marsaglia's MWC (multiply with carry) generator.
/// Although it is very simple, it passes Marsaglia's DIEHARD
/// series of random number generator tests.
/// Written by John D. Cook

inline unsigned int simple_rng_next() {
	// 0 <= u < 2^32
	unsigned int u;
	simple_rng_s_mz = 36969 * (simple_rng_s_mz & 65535) + (simple_rng_s_mz >> 16);
	simple_rng_s_mw = 18000 * (simple_rng_s_mw & 65535) + (simple_rng_s_mw >> 16);
	u = (simple_rng_s_mz << 16) + simple_rng_s_mw;
	return u;
}
