#include <stdlib.h>
#include <sys/time.h>
#include "timer.h"

static unsigned long residual_error = 0;

void init_time(void) {
	unsigned long t1, t2;
	t1 = get_time();
	t2 = get_time();
	residual_error = t2 - t1;
}

inline unsigned long get_time(void) {
	struct timeval t1;
	gettimeofday(&t1, NULL);
	return 1000000L * t1.tv_sec + t1.tv_usec;
}

inline unsigned long diff_time(unsigned long t1, unsigned long t2) {
	return t2 - t1 - residual_error;
}