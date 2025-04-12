#define _GNU_SOURCE
#include <argp.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/sched.h>
#include <pty.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <utmp.h>

#include "arguments.h"
#include "cgroups.h"

#define BUFFER_SIZE 1024

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
  while (token != NULL) {
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
    tokens[i++] = strdup(token);  // Copy each token into the array
    token = strtok(NULL, delimiter);
  }

  tokens[i] = NULL;
  *count = num_tokens;
  free(temp_string);
  return tokens;
}

void free_tokens(char** tokens, int num_tokens) {
  if (tokens == NULL) return;

  for (int i = 0; i < num_tokens; i++) {
    free(tokens[i]);
  }
  free(tokens);
}

void run_container(Arguments* args_ptr) {
  // Pids cgroup is updated on the host
  if (args_ptr->max_processes != NULL) set_pids_cgroup(args_ptr->max_processes);

  if (unshare(CLONE_NEWNS) == -1) {
    perror("unshare failed");
    exit(1);
  }

  if (chdir("/home") == -1) {
    perror("chdir failed");
    exit(1);
  }

  if (sethostname("container", sizeof("container")) == -1) {
    perror("sethostname failed");
    exit(1);
  }

  if (chroot(args_ptr->filesystem_path) == -1) {
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

  int num_tokens = 0;
  char** tokens = split_string(args_ptr->args[1], " ", &num_tokens);

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

  exit(0);
}

void run(Arguments* args_ptr) {
  struct clone_args clargs = {0};
  clargs.flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWCGROUP;
  clargs.exit_signal = SIGCHLD;

  pid_t pid = syscall(SYS_clone3, &clargs, sizeof(struct clone_args));

  if (pid == 0) {
    printf("(Child) Starting.... %d\n", getpid());
    run_container(args_ptr);
  } else if (pid > 0) {
    printf("(Parent) Waiting.... %d\n", getpid());

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
