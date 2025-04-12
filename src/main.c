#include <argp.h>

#include "arguments.h"
#include "container.h"

const char* argp_program_version = "Runix v1.0.0";
const char* doc =
    "Runix -- a minimal linux container runtime in C"
    "\v"
    "EXAMPLE USAGE: "
    "\n To run the container on a filesystem stored in fs/ubuntu-fs and limit "
    "the number of processes to 20 within the container :"
    "\n ./runix -r ./fs/ubuntu-fs -p 20 run \"/bin/bash\"";
const char* args_doc = "run <command>";
struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output", 0},
    {"rootfs", 'r', "<path>", 0,
     "Path to the POSIX compliant root filesystem to run the container on", 0},
    {"pids-limit", 'p', "<int>", 0,
     "Maximum numer of processes that can run inside the container", 0},
    {0}};

int main(int argc, char* argv[]) {
  Arguments args;

  struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL};
  argp_parse(&argp, argc, argv, 0, 0, &args);

  run(&args);

  return 0;
}