#include <argp.h>
#include "args.h"

error_t parse_opt(int key, char *arg, struct argp_state *state) {
	Arguments *args = state->input;

	switch (key) {
		case 'v':
			args->verbose = 1;
			break;

		case 'r':
			args->filesystem_path = arg;
			break;

		case ARGP_KEY_ARG:
			if (state->arg_num > 2){
				fprintf(stderr, "Error: Too many arguments %d.\n", state->arg_num);
				argp_usage(state);
			}	
			args->args[state->arg_num] = arg;
			break;
	
		case ARGP_KEY_END:
			if (state->arg_num < 2){
				fprintf(stderr, "Error: Not enough arguments.\n");
				argp_usage(state);
			}
				
			// Check if the mandatory argument was provided
			if (args->filesystem_path == NULL) {
				fprintf(stderr, "Error: Missing mandatory argument for filesystem. Pass the rootfs filesystem using -r or --rootfs\n");
				argp_usage(state); // Exit with usage message
			}
			break;
	
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

