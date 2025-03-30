#include <argp.h>
#include "args.h"

error_t parse_opt(int key, char *arg, struct argp_state *state) {
	Arguments *args = state->input;

	switch (key) {
		case 'v':
			args->verbose = 1;
			break;

		case ARGP_KEY_ARG:
			if (state->arg_num >= 2)
				/* Too many arguments. */
				argp_usage(state);
			args->args[state->arg_num] = arg;
			break;
	
		case ARGP_KEY_END:
			if (state->arg_num < 2)
				/* Not enough arguments. */
				argp_usage(state);
			break;
	
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

