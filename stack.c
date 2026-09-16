#include <stdlib.h>
#include "stack.h"

#define INITIAL_STACK_SIZE 256

static struct stack_entry *stack = NULL;
static int stack_curr_size = 0;
static int stack_capacity = 0;
static int active_workers = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int push_stack(struct stack_entry entry){
  pthread_mutex_lock(&mutex);
  if (stack_curr_size == stack_capacity){
    stack_capacity = (stack_capacity == 0) ? INITIAL_STACK_SIZE : stack_capacity * 2;
    stack = realloc(stack, stack_capacity * sizeof(struct stack_entry));
  }
  stack[stack_curr_size++] = entry;
  pthread_mutex_unlock(&mutex);
  return 0;
}

int pop_stack(struct stack_entry *output){
  int ret;
  pthread_mutex_lock(&mutex);
  if (stack_curr_size > 0){
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

void stack_free_all(void){
  free(stack);
}