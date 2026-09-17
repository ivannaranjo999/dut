#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include "size.h"

int is_regular_file(const char *path){
  struct stat path_stat;
  if (stat(path, &path_stat) != 0){
    return -1;
  }
  return S_ISREG(path_stat.st_mode);
}

int is_dir(const char *path){
  struct stat path_stat;
  if (stat(path, &path_stat) != 0){
    return -1;
  }
  return S_ISDIR(path_stat.st_mode);
}

int is_symlink(const char* path){
  struct stat path_stat;
  if (lstat(path, &path_stat) != 0){
    return -1;
  }
  return S_ISLNK(path_stat.st_mode);
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
  if (is_symlink(entry.path)) {
    return 0;
  } else if (is_regular_file(entry.path)) {
    return get_file_size(entry.path);
  } else if (is_dir(entry.path)) {
    DIR *dir = opendir(entry.path);
    if (dir == NULL){
      fprintf(stderr, "Failed to open dir %s\n", entry.path);
      return 0;
    }

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

void format_size(long kb_size, char *out, size_t out_size){
  const char *units[] = {"K", "M", "G", "T", "P"};
  double abs_size = (double)kb_size;
  int negative = kb_size < 0;
  if (negative) abs_size = -abs_size;

  int unit = 0;
  while (abs_size >= 1024 && unit < 4){
    abs_size /= 1024;
    unit++;
  }

  double human_size = negative ? -abs_size : abs_size;

  if (human_size == (long)human_size){
    snprintf(out, out_size, "%ld%s", (long)human_size, units[unit]);
  } else {
    snprintf(out, out_size, "%.2f%s", human_size, units[unit]);
  }
}