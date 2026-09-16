#ifndef ARGS_H
#define ARGS_H

extern char **given_paths;
extern int given_paths_size;
extern int is_delta;

void parse_args(int argc, char *argv[]);

#endif