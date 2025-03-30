#define _GNU_SOURCE
#include <argp.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/sched.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include "args.h"
#include <signal.h>
#include <sched.h>
#include <stdlib.h>

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

void write_resource(const char *base_path, const char *resource_name,
                    const char *resource_value) {

	char full_path[100];
	sprintf(full_path, "%s/%s", base_path, resource_name);
	FILE *file = fopen(full_path, "w");
	if (file) {
		fputs(resource_value, file);
		fclose(file);
	} else {
		printf("Unable to write resource to disk.");
		exit(1);
  	}
}

void set_cgroup() {

	const char *base_path = "/sys/fs/cgroup";
	const char *resource_folder = "pids";
	const char *cgroup_name = "runix";
	char path[100];

	sprintf(path, "%s/%s/%s", base_path, resource_folder, cgroup_name);
	if (mkdir(path, 0755) == -1) {
		if (errno != EEXIST) {
			perror("mkdir failed");
			exit(1);
		}
	}

	write_resource(path, "pids.max", "20");

	write_resource(path, "notify_on_release", "1");

	char pid_str[10];
	sprintf(pid_str, "%d", getpid());
	write_resource(path, "cgroup.procs", pid_str);
}

char** split_string(const char* input, const char* delimiter, int* count) {
    // Create duplicate string
	char* temp_string = strdup(input);
    if (!temp_string) {
        perror("Failed to allocate memory");
        exit(1);
    }

	// Count number of tokens
	int num_tokens = 0;
	char* token = strtok(temp_string, delimiter);
	while(token != NULL){
		num_tokens++;
		token = strtok(NULL, delimiter);
	}

    // Allocate memory for the array of tokens
    char** tokens = malloc(sizeof(char*) * (num_tokens + 1)); 
    if (!tokens) {
        perror("Failed to allocate memory");
		free(temp_string);
        exit(1);
    }

    token = strtok(temp_string, delimiter);
    int i = 0;
    while (token != NULL) {
        tokens[i++] = strdup(token); // Copy each token into the array
        token = strtok(NULL, delimiter);
    }

    tokens[i] = NULL; 
    *count = num_tokens; 
    free(temp_string); 
    return tokens;
}

void free_tokens(char** tokens, int num_tokens){
	if(tokens == NULL)
		return;

	for(int i = 0 ; i < num_tokens; i++){
		free(tokens[i]);
	}
	free(tokens);
}

void run_container(Arguments* args_ptr) {

	set_cgroup();

	if (unshare(CLONE_NEWNS) == -1) {
		perror("unshare failed");
		exit(1);
	}

	if (chdir("/home") == -1) {
		perror("chdir failed");
		exit(1);
	}

	if (setsid() == -1) {
		perror("setsid failed");
		exit(1);
	}

	if (sethostname("container", sizeof("container")) == -1) {
		perror("sethostname failed");
		exit(1);
	}

	if (chroot("/home/raga/repos/linux-container-from-scratch/ubuntu-fs") == -1) {
		perror("chroot failed");
		exit(1);
	}
	if (chdir("/") == -1) {
		perror("chdir failed");
		exit(1);
	}

	if (mount("proc", "proc", "proc", 0, NULL) == -1) {
		perror("proc mount failed");
		exit(1);
	}

	if (mount("devpts", "/dev/pts", "devpts", 0, NULL) == -1) {
		perror("devpts mount failed");
		exit(1);
	}

	int num_tokens = 0;
	char** tokens = split_string(args_ptr->args[1], " ",&num_tokens);
	if (execv(tokens[0], tokens) == -1) {
		perror("(Child) execv failed");
		free_tokens(tokens, num_tokens);
		exit(1);
	}
	
	free_tokens(tokens, num_tokens);

	printf("(Child) PID %d finished executing command.\n", getpid());
	
	if (umount("proc") == -1) {
		perror("unmount failed");
		exit(1);
	}

	if (umount("devpts") == -1) {
		perror("devpts failed");
		exit(1);
	}

	exit(0);
}

void run(Arguments* args_ptr) {

	struct clone_args clargs = {0};
	clargs.flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWCGROUP;
	clargs.exit_signal = SIGCHLD;

	pid_t pid = syscall(SYS_clone3, &clargs, sizeof(struct clone_args));

	if (pid == 0) {
		printf("(Child) Starting.... %d\n",  getpid());
		run_container(args_ptr);
	} else if (pid > 0) {
		printf("(Parent) Waiting.... %d\n",  getpid());
		int status;
		waitpid(pid, &status, 0);

		if (WIFEXITED(status)) {
			printf("(Parent) Child exited with status %d\n", WEXITSTATUS(status));
		}
		exit(0);
	} else {
		// Negative pid (-1) means child was not created
		perror("(Parent) Could not create child. Reason: ");
		exit(-1);
	}
}


const char* argp_program_version = "Runix 1.0.0";
const char* argp_program_bug_address = "<ragavendrams45@gmail.com>";
const char* doc = "Runix -- a linux container runtime in C";
const char* args_doc = "run <command to run inside container>";
struct argp_option options[] = {
  {"verbose", 'v', 0, 0, "Produce verbose output"}, {0}};

int main(int argc, char *argv[]) {

  Arguments args;
  args.verbose = 0;

  struct argp argp = {options, parse_opt, args_doc, doc};
  argp_parse(&argp, argc, argv, 0, 0, &args);

  printf("ARG1 = %s\nARG2 = %s\n"
         "VERBOSE = %s\n",
         args.args[0], args.args[1],
         args.verbose ? "yes" : "no");

  run(&args);

  return 0;
}