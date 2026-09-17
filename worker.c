#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include "worker.h"
#include "stack.h"
#include "size.h"
#include "progress.h"

#define WORKER_YIELD 1000

long *total_per_argument;

void* worker_main(void * param){
  (void)param;
  long *local_totals = calloc(given_paths_size, sizeof(long));

  struct stack_entry entry;
  for (;;){
    int ret = pop_stack(&entry);
    if (ret == 0){
      local_totals[entry.index] += get_size(entry);
      free(entry.path);
      atomic_fetch_add(&files_done, 1);
      finish_work();
    } else {
      if (should_exit()){
        break;
      }
      usleep(WORKER_YIELD);
    }
  }

  pthread_mutex_lock(&mutex);
  for (int i = 0; i < given_paths_size; ++i){
    total_per_argument[i] += local_totals[i];
  }
  pthread_mutex_unlock(&mutex);

  return NULL;
}