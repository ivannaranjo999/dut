#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>

#define INITIAL_STACK_SIZE 256
#define WORKER_YIELD 1000

/* Stack info */
struct stack_entry {
  char *path;
  int index;
};
struct stack_entry *stack = NULL;
int stack_curr_size = 0;
int stack_capacity = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* array for size of each argument */
long *total_per_argument;
/* array for working thread or not */
long active_workers = 0;

/* number of cores */
long n_cores;

int push_stack(struct stack_entry entry){
  pthread_mutex_lock(&mutex);
  if(stack_curr_size == stack_capacity){
    stack_capacity = (stack_capacity == 0) ? INITIAL_STACK_SIZE : stack_capacity * 2;
    stack = realloc(stack, stack_capacity * sizeof(struct stack_entry));
  }
  stack[stack_curr_size++] = entry;
  pthread_mutex_unlock(&mutex);
  return 0;
}

int pop_stack(struct stack_entry * output){
  int ret;
  pthread_mutex_lock(&mutex);
  if(stack_curr_size > 0){
    int top = stack_curr_size - 1;
    *output = stack[top];
    stack_curr_size--;
    active_workers++;
    ret = 0;
  } else {
    ret = -1;
  }
  pthread_mutex_unlock(&mutex);
  return ret;
}

void finish_work(void){
  pthread_mutex_lock(&mutex);
  active_workers--;
  pthread_mutex_unlock(&mutex);
}

int should_exit(void){
  int result;
  pthread_mutex_lock(&mutex);
  result = (stack_curr_size == 0 && active_workers == 0);
  pthread_mutex_unlock(&mutex);
  return result;
}

void format_size(long kb_size, char *out, size_t out_size){
  const char *units[] = {"K", "M", "G", "T", "P"};
  double human_size = (double)kb_size;
  int unit = 0;

  while (human_size >= 1024 && unit < 4){
    human_size /= 1024;
    unit++;
  }

  if (human_size == (long)human_size){
    snprintf(out, out_size, "%ld%s", (long)human_size, units[unit]);
  } else {
    snprintf(out, out_size, "%.2f%s", human_size, units[unit]);
  }
}

int is_regular_file(const char *path){
  struct stat path_stat;
  if(stat(path, &path_stat) != 0){
    return -1;
  }
  return S_ISREG(path_stat.st_mode);
}

int is_dir(const char *path){
  struct stat path_stat;
  if(stat(path, &path_stat) != 0){
    return -1;
  }
  return S_ISDIR(path_stat.st_mode);
}

long get_file_size(const char *path){
  struct stat st;

  if (stat(path, &st) != 0){
    fprintf(stderr, "Cannot get size of %s\n", path);
    return -1;
  }

  return st.st_blocks / 2; // 1 KB
}

long get_size(struct stack_entry entry){
  if (is_regular_file(entry.path)) {
    /* Is file */
    return get_file_size(entry.path);
  } else if(is_dir(entry.path)){ 
    /* Is dir */
    DIR *dir = opendir(entry.path);
    if (dir == NULL){
      fprintf(stderr, "Failed to open dir %s\n", entry.path);
      return 0;
    }

    long total = 0;
    struct dirent *dir_entry;

    while ((dir_entry = readdir(dir)) != NULL) {
      if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) {
        continue;
      }

      char child_path[PATH_MAX];
      snprintf(child_path, sizeof(child_path), "%s/%s", entry.path, dir_entry->d_name);

      struct stack_entry to_push;
      to_push.index = entry.index;
      to_push.path = strdup(child_path);

      push_stack(to_push);
    }

    closedir(dir);
    return 0;
  } else { 
    /* Is something else */
    fprintf(stderr, "%s's type not supported\n", entry.path);
    return 0; 
  }
}

void* worker_main(void * param){
  int ret;
  int thread_id = *(int*) param;
  struct stack_entry entry;
  for(;;){
    ret = pop_stack(&entry);
    if (ret == 0){
      total_per_argument[entry.index] += get_size(entry);
      free(entry.path);
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

int main(int argc, char *argv[]){
  n_cores = sysconf(_SC_NPROCESSORS_ONLN);
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

  for(int idx = 0; idx < n_dirs; ++idx){
    struct stack_entry entry;
    entry.index=idx;
    if (argc == 1){
      entry.path=strdup(stdin_paths[idx]);
      push_stack(entry);
    } else {
      entry.path=strdup(argv[idx+1]);
      push_stack(entry);
    }
  }

  pthread_t threads[n_cores];
  int *thread_ids = malloc(n_cores * sizeof(int));

  for(int t = 0; t < n_cores; ++t){
    thread_ids[t] = t;
    pthread_create(&threads[t], NULL, worker_main, &thread_ids[t]);
  }

  for(int t = 0; t < n_cores; ++t){
    pthread_join(threads[t], NULL);
  }
  
  for(int idx = 0; idx < n_dirs; ++idx){
    char size_str[32];
    format_size(total_per_argument[idx], size_str, sizeof(size_str));

    if (argc == 1){
      printf("%-10s %s\n", size_str, stdin_paths[idx]);
    } else {
      printf("%-10s %s\n", size_str, argv[idx+1]);
    }
  }

  if (argc == 1) {
    for (int i = 0; i < n_dirs; ++i) {
      free(stdin_paths[i]);
    }
    free(stdin_paths);
  }

  free(stack);
  free(thread_ids);
  free(total_per_argument);

  return 0;
}
