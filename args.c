#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "args.h"

char **given_paths = NULL;
int given_paths_size = 0;
int is_delta = 0;

void print_help(){
  printf("dut, tool by Ivan Naranjo Ortega.\n");
  printf("Usage: dut [ARGS] [PATHS].\n\n");
  printf("Arguments, none are mandatory:\n");
  printf("\t--help:  displays this help.\n");
  printf("\t--delta: shows difference in size of given paths if a previous snapshot is stored.\n");
  exit (0);
}

char ** extract_args_from_stdin(int * count){
  char **stdin_paths = NULL;
  int capacity = 16;
  stdin_paths = malloc(capacity * sizeof(char*));
  char buf[PATH_MAX];
  
  while (scanf("%1023s", buf) == 1) {
    if (*count == capacity) {
    capacity *= 2;
    stdin_paths = realloc(stdin_paths, capacity * sizeof(char*));
    }
    stdin_paths[(*count)++] = strdup(buf);
  }

  return stdin_paths;
}

void detect_argument(int length, char *arguments[]){
  for (int i = 0; i < length; ++i){
    if(strcmp(arguments[i], "--help") == 0){
        print_help();
    } else if(strcmp(arguments[i], "--delta") == 0){
        is_delta = 1;
    } else {
      given_paths[given_paths_size++] = strdup(arguments[i]);
    }
  } 
}

void parse_args(int argc, char *argv[]){
  char **cli_args = (argc > 1) ? argv + 1 : NULL;
  int cli_count = (argc > 1) ? argc - 1 : 0;

  given_paths = malloc(sizeof(char *) * (cli_count > 0 ? cli_count : 1));
  detect_argument(cli_count, cli_args);

  if (given_paths_size == 0 && !isatty(fileno(stdin))) {
    int count = 0;
    char **stdin_paths = extract_args_from_stdin(&count);

    given_paths = realloc(given_paths, sizeof(char *) * (given_paths_size + count));
    for (int i = 0; i < count; ++i){
      given_paths[given_paths_size++] = stdin_paths[i];
    }
    free(stdin_paths);
  }

}