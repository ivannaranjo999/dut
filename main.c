#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "stack.h"
#include "size.h"
#include "worker.h"
#include "progress.h"
#include "args.h"

int main(int argc, char *argv[]){
  long n_cores = sysconf(_SC_NPROCESSORS_ONLN);

  parse_args(argc, argv);

  total_per_argument = calloc(given_paths_size, sizeof(long));

  for (int idx = 0; idx < given_paths_size; ++idx){
    struct stack_entry entry;
    entry.index = idx;
    entry.path = strdup(given_paths[idx]);
    push_stack(entry);
  }

  pthread_t threads[n_cores];
  pthread_t progress_thread;

  for (int t = 0; t < n_cores; ++t){
    pthread_create(&threads[t], NULL, worker_main, NULL);
  }

  pthread_create(&progress_thread, NULL, progress_main, NULL);

  for (int t = 0; t < n_cores; ++t){
    pthread_join(threads[t], NULL);
  }

  progress_signal_done();
  pthread_join(progress_thread, NULL);

  for (int idx = 0; idx < given_paths_size; ++idx){
    char size_str[32];
    format_size(total_per_argument[idx], size_str, sizeof(size_str));
    printf("%-10s %s\n", size_str, given_paths[idx]);

    free(given_paths[idx]);
  }
  free(given_paths);

  stack_free_all();
  free(total_per_argument);

  return 0;
}