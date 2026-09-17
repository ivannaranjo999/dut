#ifndef DELTA_H
#define DELTA_H

extern char **given_paths;
extern long *total_per_argument;

int print_delta(int index, time_t ts, long * diff);

#endif