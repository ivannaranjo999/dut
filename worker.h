#ifndef WORKER_H
#define WORKER_H

/* Size per argument given to dut */
extern long *total_per_argument;

void* worker_main(void *param);

#endif