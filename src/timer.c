#include <stdlib.h>
#include <sys/time.h>
#include "timer.h"

static uint64_t residual_error = 0;

void init_time(void) {
	unsigned long t1, t2;
	t1 = get_time();
	t2 = get_time();
	residual_error = t2 - t1;
}

inline uint64_t get_time(void) {
	struct timeval t1;
	gettimeofday(&t1, NULL);
	return 1000000L * t1.tv_sec + t1.tv_usec;
}

inline uint64_t diff_time(uint64_t t1, uint64_t t2) {
	return t2 - t1 - residual_error;
}
