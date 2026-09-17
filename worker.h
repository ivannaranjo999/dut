#ifndef WORKER_H
#define WORKER_H

/* Size per argument given to dut */
extern long *total_per_argument;

extern int given_paths_size;

void* worker_main(void * param);

#endif