#ifndef CLOCK_H_
#define CLOCK_H_

typedef unsigned int clock_time_t;

#define CLOCK_SECOND (clock_time_t)100

void clock_init(void);
clock_time_t clock_time(void);
unsigned long clock_seconds(void);

#endif /* CLOCK_H_ */
