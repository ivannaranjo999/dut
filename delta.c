#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include "delta.h"

void create_dot_dut_dir(){
  char dut_dir[PATH_MAX];
  snprintf(dut_dir, sizeof(dut_dir), "%s/.dut/", getenv("HOME"));

  DIR *d = opendir(dut_dir);
  if (d != NULL) {
    closedir(d);
    return;
  }

  if (errno == ENOENT) {
    mkdir(dut_dir, 0755);
  }
}

unsigned long get_hash(const char * path){
  unsigned long hash = 0;
  while(*path) {
    hash = hash * 31 + (unsigned char)(*path);
    path++;
  }
  return hash;
}

unsigned long get_absolute_path_hash(const char *relative_path){
  char full_path[PATH_MAX];

  if (realpath(relative_path, full_path) == NULL) {
    perror("realpath");
    return -1;
  }

  return get_hash(full_path);
}

void write_delta_file(int index, time_t ts){
  create_dot_dut_dir();

  unsigned long hash = get_absolute_path_hash(given_paths[index]);

  char dut_dir[PATH_MAX];
  snprintf(dut_dir, sizeof(dut_dir), "%s/.dut/", getenv("HOME"));

  char hash_filename[PATH_MAX + 32];
  int written = snprintf(hash_filename, sizeof(hash_filename), "%s%lu.txt", dut_dir, hash);
  if (written < 0 || (size_t)written >= sizeof(hash_filename)) {
    fprintf(stderr, "hash filename truncated\n");
    return;
  }

  FILE *f = fopen(hash_filename, "a");
  if (f == NULL) {
    perror("fopen");
    return;
  }

  fprintf(f, "%ld %ld\n", ts, total_per_argument[index]);
  fclose(f);
}