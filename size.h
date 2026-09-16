#ifndef SIZE_H
#define SIZE_H

#include <stddef.h>
#include "stack.h"

int is_regular_file(const char *path);
int is_dir(const char *path);
int is_symlink(const char *path);
long get_file_size(const char *path);
long get_size(struct stack_entry entry);
void format_size(long kb_size, char *out, size_t out_size);

#endif