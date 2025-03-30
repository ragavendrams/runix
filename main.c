#include <argp.h>
#include "args.h"
#include "container.h"

/*
Create Linux Containers from scratch
The container should:
- Accept similar commands docker i.e ./linux-container run <linux command to run
inside container>
- Be a process that is isolated from the host (processes running on host are not
visible to container and vice versa)
- Have its own filesystem
- be able to limit its resource usage (CPU, memory)
*/

const char* argp_program_version = "Runix 1.0.0";
const char* argp_program_bug_address = "<ragavendrams45@gmail.com>";
const char* doc = "Runix -- a linux container runtime in C";
const char* args_doc = "run <command to run inside container>";
struct argp_option options[] = {
  {"verbose", 'v', 0, 0, "Produce verbose output"}, 
  {"rootfs", 'r', 0, 0, "Path to the POSIX compliant rootfs to run the container on"},{0}};

int main(int argc, char *argv[]) {

  Arguments args;
  args.verbose = 0;
  args.filesystem_path = NULL;
  
  struct argp argp = {options, parse_opt, args_doc, doc};
  argp_parse(&argp, argc, argv, 0, 0, &args);

  printf("ARG1 = %s\nARG2 = %s\nVERBOSE = %s\nFILESYSTEM = %s\n",
         args.args[0], args.args[1],
         args.verbose ? "yes" : "no", 
		 (args.filesystem_path == NULL) ? "Rootfs not passed in.": args.filesystem_path);

  run(&args);

  return 0;
}