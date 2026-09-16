#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include "worker.h"
#include "stack.h"
#include "size.h"
#include "progress.h"

#define WORKER_YIELD 1000

long *total_per_argument;

void* worker_main(void *param){
  (void)param;

  struct stack_entry entry;
  for (;;){
    int ret = pop_stack(&entry);
    if (ret == 0){
      total_per_argument[entry.index] += get_size(entry);
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
  return NULL;
}