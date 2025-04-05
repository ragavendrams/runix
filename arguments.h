#ifndef RUNIX_INCLUDE_ARGS_H
#define RUNIX_INCLUDE_ARGS_H
#include <argp.h>
#include <stdbool.h>

typedef struct {
	char *args[2]; /* arg1 & arg2 */
	bool verbose;
	char* filesystem_path;
	char* max_processes; // use char* instead of int because 'max' is also a valid value for pids.max
} Arguments;

error_t parse_opt(int key, char *arg, struct argp_state *state);

#endif