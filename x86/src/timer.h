#ifndef __TIMER_H
#define __TIMER_H

void init_time(void);
inline unsigned long get_time(void);
inline unsigned long diff_time(unsigned long t1, unsigned long t2);

#endif