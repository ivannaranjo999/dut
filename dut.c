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

struct worker_params {
  int count;
  char **paths; 
};

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

long get_size(const char *path){
  if (is_regular_file(path)) {
    /* Is file */
    return get_file_size(path);
  } else if(is_dir(path)){ 
    /* Is dir */
    DIR *dir = opendir(path);
    if (dir == NULL){
      fprintf(stderr, "Failed to open dir %s\n", path);
      return 0;
    }

    long total = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        continue;
      }

      char child_path[PATH_MAX];
      snprintf(child_path, sizeof(child_path), "%s/%s", path, entry->d_name);

      total += get_size(child_path);
    }

    closedir(dir);
    return total;
  } else { 
    /* Is something else */
    fprintf(stderr, "%s's type not supported\n", path);
    return 0; 
  }
}

void* worker_main(void* param){
  struct worker_params *wp = (struct worker_params*)param;
  const char *units[] = {"K", "M", "G", "T", "P"};

  for (int i = 0; i < wp->count; ++i){
    long kb_size = get_size(wp->paths[i]);
    double human_size = (double)kb_size;
    int unit = 0; 
    while (human_size >= 1024 && unit < 4){
      human_size /= 1024; 
      unit++;
    }


    if (human_size > 0){
      char size_str[32];
      if (human_size == (long)human_size){
        snprintf(size_str, sizeof(size_str), "%ld%s", (long)human_size, units[unit]);
      } else {
        snprintf(size_str, sizeof(size_str), "%.2f%s", human_size, units[unit]);
      }

#ifdef DUT_DEBUG
      printf("%-10s %-40s %d\n", size_str, wp->paths[i], (int)syscall(SYS_gettid));
#else
      printf("%-10s %s\n", size_str, wp->paths[i]);
#endif
    }
  }

  /* For malloc in main process */
  free(wp);
  return NULL;
}

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

  int per_thread=(n_dirs + n_cores - 1) / n_cores;
  char *paths[n_cores][per_thread];
  int counts[n_cores];
  memset(counts, 0, sizeof(counts));

  for(int idx = 0; idx < n_dirs; ++idx){
    int target_thread = idx % n_cores; 
    int pos = counts[target_thread]++;
    paths[target_thread][pos] = (argc == 1) ? stdin_paths[idx] : argv[idx + 1];
  }

  pthread_t threads[n_cores];

  for(int t = 0; t < n_cores; ++t){
    struct worker_params *wp = malloc(sizeof(struct worker_params));
    wp->count = counts[t];
    wp->paths = paths[t];

    pthread_create(&threads[t], NULL, worker_main, wp);
  }

  for(int t = 0; t < n_cores; ++t){
    pthread_join(threads[t], NULL);
  }

  if (argc == 1) {
    for (int i = 0; i < n_dirs; ++i) {
      free(stdin_paths[i]);
    }
    free(stdin_paths);
  }

  return 0;
}
