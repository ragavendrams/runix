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
#include <signal.h>
#include <sched.h>
#include <stdlib.h>
#include <pty.h>
#include <termios.h>
#include <utmp.h>
#include "cgroups.h"
#include "arguments.h"


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
	
	free(temp_string);

	// Allocate memory for the array of tokens
	char** tokens = malloc(sizeof(char*) * (num_tokens + 1)); 
	if (!tokens) {
		perror("Failed to allocate memory");
		exit(1);
	}

	temp_string = strdup(input);
    if (!temp_string) {
        perror("Failed to allocate memory");
        free(tokens);
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

int open_pty(int *slave_fd, char **slave_name) {

	printf("1\n");

    int master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1) {
        return -1;
    }

	printf("2\n");
	
    if (grantpt(master_fd) == -1 || unlockpt(master_fd) == -1) {
        close(master_fd);
        return -1;
    }

	printf("3\n");
	
    const char *slave_path = ptsname(master_fd);
    if (slave_path == NULL) {
        close(master_fd);
        return -1;
    }

	printf("4\n");
	
    *slave_fd = open(slave_path, O_RDWR | O_NOCTTY);
    if (*slave_fd == -1) {
        close(master_fd);
        return -1;
    }
	printf("5\n");
	
    *slave_name = strdup(slave_path); // Allocate memory for the slave name
	printf("6 %d\n", master_fd);

    return master_fd;
}

void run_container(Arguments* args_ptr) {

	printf("10\n");

	if(args_ptr->max_processes != NULL)
		set_pids_cgroup(args_ptr->max_processes);

	printf("11\n");
	
	if (unshare(CLONE_NEWNS) == -1) {
		perror("unshare failed");
		exit(1);
	}

	printf("12\n");
	
	if (chdir("/home") == -1) {
		perror("chdir failed");
		exit(1);
	}
	printf("13\n");
	
	if (setsid() == -1) {
		perror("setsid failed");
		exit(1);
	}
	printf("14\n");
	
	if(sethostname("container", sizeof("container")) == -1) {
		perror("sethostname failed");
		exit(1);
	}
	printf("15\n");
	
	if (chroot(args_ptr->filesystem_path) == -1) {
		perror("chroot failed");
		exit(1);
	}
	printf("16\n");
	
	if (chdir("/") == -1) {
		perror("chdir failed");
		exit(1);
	}
	printf("17\n");
	
	if (mount("proc", "proc", "proc", 0, NULL) == -1) {
		perror("proc mount failed");
		exit(1);
	}
	printf("18\n");
	
	if (mount("devpts", "/dev/pts", "devpts", 0, "gid=5,mode=620") == -1) {
		perror("devpts mount failed");
		exit(1);
	}
	printf("19\n");
	
	int master_fd, slave_fd;
    char *slave_name;

    master_fd = open_pty(&slave_fd, &slave_name);
	printf("20\n");
    if (master_fd == -1) {
        perror("open_pty failed");
        // Handle error
    } else {
		for(int i = 0; i < 20; i++){
			int flags = fcntl(i, F_GETFD);
			if(flags != -1){
				printf("File Descriptor %d is open\n", i);
			}
		}
        if (dup2(slave_fd, STDIN_FILENO) == -1) {
			perror("dup2 STDIN failed");
			exit(1);
		}
		if (dup2(slave_fd, STDOUT_FILENO) == -1) {
			perror("dup2 STDOUT failed");
			exit(1);
		}
		if (dup2(slave_fd, STDERR_FILENO) == -1) {
			perror("dup2 STDERR failed");
			exit(1);
		}
		//close(slave_fd);
		if(close(slave_fd) == -1){
			perror("close slave_fd failed");
			exit(1);
		}
        if(close(master_fd) == -1){
			perror("close master_fd failed");
			exit(1);
		}
    }
	//struct termios term;
    //if (tcgetattr(0, &term) == 0) { // Get attributes from stdin
    //    tcsetattr(0, TCSANOW, &term); // Apply to stdin (slave PTY)
    //    tcsetattr(1, TCSANOW, &term); // Apply to stdout
    //    tcsetattr(2, TCSANOW, &term); // Apply to stderr
    //}

	printf("Crossed open_pty\n");
	fflush(stdout);

	int num_tokens = 0;
	char** tokens = split_string(args_ptr->args[1], " ",&num_tokens);
	printf("%d\n", num_tokens);
	for(int i = 0 ; i < num_tokens; i++){
		printf("%s ", tokens[i]);
	}
	fflush(stdout);

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

