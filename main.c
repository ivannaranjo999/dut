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

int main(int argc, char *argv[]){
  long n_cores = sysconf(_SC_NPROCESSORS_ONLN);
  int n_dirs = argc - 1;
  char **stdin_paths = NULL;

  if (argc == 1) {
    int capacity = 16;
    int count = 0;
    stdin_paths = malloc(capacity * sizeof(char*));
    char buf[PATH_MAX];

    while (scanf("%1023s", buf) == 1) {
      if (count == capacity) {
        capacity *= 2;
        stdin_paths = realloc(stdin_paths, capacity * sizeof(char*));
      }
      stdin_paths[count++] = strdup(buf);
    }

    n_dirs = count;
  } else {
    n_dirs = argc - 1;
  }

  total_per_argument = calloc(n_dirs, sizeof(long));

  for (int idx = 0; idx < n_dirs; ++idx){
    struct stack_entry entry;
    entry.index = idx;
    if (argc == 1){
      entry.path = strdup(stdin_paths[idx]);
    } else {
      entry.path = strdup(argv[idx + 1]);
    }
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

  for (int idx = 0; idx < n_dirs; ++idx){
    char size_str[32];
    format_size(total_per_argument[idx], size_str, sizeof(size_str));

    if (argc == 1){
      printf("%-10s %s\n", size_str, stdin_paths[idx]);
    } else {
      printf("%-10s %s\n", size_str, argv[idx + 1]);
    }
  }

  if (argc == 1) {
    for (int i = 0; i < n_dirs; ++i) {
      free(stdin_paths[i]);
    }
    free(stdin_paths);
  }

  stack_free_all();
  free(total_per_argument);

  return 0;
}