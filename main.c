#include <argp.h>
#include "arguments.h"
#include "container.h"

/*
Create Linux Containers from scratch
The container should:
- Accept similar commands docker i.e ./linux-container run <command to run inside container>
- Be a process that is isolated from the host (processes running on host are not visible to container and vice versa)
- Have its own filesystem
- be able to limit its resource usage (CPU, memory)
*/

const char* argp_program_version = "Runix 1.0.0";
const char* argp_program_bug_address = "<ragavendrams45@gmail.com>";
const char* doc = "Runix -- a minimal linux container runtime in C";
const char* args_doc = "run <command to run inside container>";
struct argp_option options[] = {
  {"verbose", 'v', 0, 0, "Produce verbose output", 0}, 
  {"rootfs", 'r', "FILESYSTEM", 0, "Path to the POSIX compliant rootfs to run the container on", 0},
  {"pids-limit", 'p', "MAX PROCESSES", 0, "Maximum numer of processes that can run inside the container", 0},{0}};

int main(int argc, char *argv[]) {

  Arguments args;
  
  struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};
  argp_parse(&argp, argc, argv, 0, 0, &args);

  run(&args);

  return 0;
}