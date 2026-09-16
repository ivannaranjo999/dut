#ifndef DELTA_H
#define DELTA_H

extern char **given_paths;
extern long *total_per_argument;

void write_delta_file(int index, time_t ts);

#endif