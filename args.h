#ifndef RUNIX_INCLUDE_ARGS_H
#define RUNIX_INCLUDE_ARGS_H
#include <argp.h>

typedef struct {
	char *args[2]; /* arg1 & arg2 */
	int verbose;
	char* filesystem_path;
} Arguments;

error_t parse_opt(int key, char *arg, struct argp_state *state);

#endif