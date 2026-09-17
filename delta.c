#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include "delta.h"
#include "size.h"

#define NUM_OF_LINES 2

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
    return 0;
  }

  return get_hash(full_path);
}

char * write_delta_file(int index, time_t ts){
  create_dot_dut_dir();

  unsigned long hash = get_absolute_path_hash(given_paths[index]);
  if (hash == 0) return NULL;

  char dut_dir[PATH_MAX];
  snprintf(dut_dir, sizeof(dut_dir), "%s/.dut/", getenv("HOME"));

  size_t hash_filename_size = (size_t)(PATH_MAX + 32);
  char *hash_filename = malloc(hash_filename_size);
  if(hash_filename == NULL) return NULL;
  int written = snprintf(hash_filename, hash_filename_size, "%s%lu.txt", dut_dir, hash);
  if (written < 0 || (size_t)written >= hash_filename_size) {
    fprintf(stderr, "hash filename truncated\n");
    return NULL;
  }

  FILE *f = fopen(hash_filename, "a");
  if (f == NULL) {
    perror("fopen");
    return NULL;
  }

  fprintf(f, "%ld %ld\n", (signed long)ts, total_per_argument[index]);
  fclose(f);

  return hash_filename;
}

char *read_range(FILE *f, long start, long end) {
    long len = end - start;
    if (len < 0) return NULL;

    char *buf = malloc((size_t)len + 1);
    if (!buf) return NULL;

    if (fseek(f, start, SEEK_SET) != 0) {
        free(buf);
        return NULL;
    }

    size_t r = fread(buf, 1, (size_t)len, f);
    buf[r] = '\0';
    return buf;
}

char **extract_last_N_lines(FILE *f, int n, int *out_count) {
  if (!f || n <= 0) return NULL;

  char **lines = calloc((size_t)n, sizeof(char *));
  if (!lines) return NULL;

  if (fseek(f, 0, SEEK_END) != 0) {
    free(lines);
    return NULL;
  }

  long file_size = ftell(f);
  if (file_size < 0) {
    free(lines);
    return NULL;
  }
  if (file_size == 0) {
    if (out_count) *out_count = 0;
    return lines;
  }

  long end = file_size;
  long pos = file_size - 1;
  int found = 0;
  int ok = 1;

  // Skip a single trailing newline
  if (fseek(f, pos, SEEK_SET) == 0) {
    int c = fgetc(f);
    if (c == '\n') {
      end = pos;
      pos--;
    }
  }

  for(; (ok) && (found < n) && (pos >= -1); --pos){
    if (pos < 0) {
      // Start of file reached without finding all requested lines
      char *buf = read_range(f, 0, end);
      if (!buf) { ok = 0; break; }
      lines[n - 1 - found] = buf;
      found++;
      break;
    }

    if (fseek(f, pos, SEEK_SET) != 0) { ok = 0; break; }
    int c = fgetc(f);

    if (c == '\n') {
      long start = pos + 1;
      char *buf = read_range(f, start, end);
      if (!buf) { ok = 0; break; }
      lines[n - 1 - found] = buf;
      found++;
      end = pos;
    }
  }

  if (!ok) {
    for (int i = 0; i < n; i++) free(lines[i]);
    free(lines);
    return NULL;
  }

  if (out_count) *out_count = found;
  return lines;
}

void free_lines(char **lines, int n){
  if (!lines) return;
  for (int i = 0; i < n; ++i) free(lines[i]);
  free(lines);
}

int print_delta(int index, time_t ts, long * diff){
  char * delta_fn = write_delta_file(index, ts);

  if (delta_fn != NULL){
    FILE *f = fopen(delta_fn, "r");
    if (f == NULL) return -1;
    int count = 0;
    char **lines = extract_last_N_lines(f, NUM_OF_LINES, &count);
    fclose(f);
    free(delta_fn);

    if((lines == NULL) || (count != NUM_OF_LINES)) return -1;

    long curr_ts[NUM_OF_LINES];
    long curr_size[NUM_OF_LINES];
    for (int i = 0; i < count; ++i){
      if(sscanf(lines[i], "%ld %ld", &curr_ts[i], &curr_size[i]) != 2) return -1;
    }
    free_lines(lines, NUM_OF_LINES);

    *diff = curr_size[1] - curr_size[0];
    if (*diff){
      char size_str[32];
      format_size(*diff, size_str, sizeof(size_str));
      printf("%s %s\n", size_str, given_paths[index]);
    }

    return 0;
  }

  return -1;
}