#ifndef STACK_H
#define STACK_H

#include <pthread.h>

struct stack_entry {
  char *path;
  int index;
};

extern pthread_mutex_t mutex;

int push_stack(struct stack_entry entry);
int pop_stack(struct stack_entry *output);
void finish_work(void);
int should_exit(void);
void stack_free_all(void);
#endif