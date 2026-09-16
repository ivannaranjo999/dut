#ifndef PROGRESS_H
#define PROGRESS_H

#include <pthread.h>
#include <stdatomic.h>

extern pthread_cond_t progress_cond;
extern _Atomic long files_done;
extern _Atomic int work_finished;

void* progress_main(void *param);
void progress_signal_done(void);

#endif