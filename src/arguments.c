#define _GNU_SOURCE
#include "arguments.h"

#include <argp.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

error_t parse_opt(int key, char *arg, struct argp_state *state) {
  Arguments *args = state->input;

  switch (key) {
    case 'v':
      args->verbose = true;
      break;

    case 'r':
      if ((args->filesystem_path = realpath(arg, NULL)) == NULL) {
        log_error("Could not resolve path to filesystem: %s", strerror(errno));
        exit(1);
      }
      break;

    case 'p':
      args->max_processes = arg;
      break;

    case ARGP_KEY_INIT:
      args->verbose = false;
      args->filesystem_path = NULL;
      args->max_processes =
          NULL;  // NULL is just easier to check instead of the default 'max'
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num > 2) {
        log_error("Error: Too many arguments %d.\n", state->arg_num);
        argp_usage(state);
      }
      args->args[state->arg_num] = arg;
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 2) {
        log_error("Error: Not enough arguments.\n");
        argp_usage(state);
      }

      // Check if the mandatory argument was provided
      if (args->filesystem_path == NULL) {
        log_error(
            "Error: Missing mandatory argument for filesystem. Pass the "
            "rootfs filesystem using -r or --rootfs\n");
        argp_usage(state);  // Exit with usage message
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}
