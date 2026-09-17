#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "progress.h"
#include "stack.h"

pthread_cond_t progress_cond = PTHREAD_COND_INITIALIZER;
_Atomic long files_done = 0;
_Atomic int work_finished = 0;

void* progress_main(void *param){
  (void)param;
  int wrote = 0;

  if (!isatty(fileno(stderr))){
    return NULL;
  }

  pthread_mutex_lock(&mutex);
  while (!work_finished){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1;

    /* Release mutex and sleep until 1 second passes or progress_cond fires */
    pthread_cond_timedwait(&progress_cond, &mutex, &ts);

    if (!work_finished){
      wrote = 1;
      fprintf(stderr, "\r%ld files analyzed...", files_done);
      fflush(stderr);
    }
  }
  pthread_mutex_unlock(&mutex);

  if (wrote) fprintf(stderr, "\n");
  fflush(stderr);
  return NULL;
}

void progress_signal_done(void){
  pthread_mutex_lock(&mutex);
  work_finished = 1;
  pthread_cond_signal(&progress_cond);
  pthread_mutex_unlock(&mutex);
}