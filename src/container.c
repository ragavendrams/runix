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
#include "log.h"

void free_tokens(char** tokens, int num_tokens) {
  if (tokens == NULL) return;

  for (int i = 0; i < num_tokens; i++) {
    free(tokens[i]);
  }
  free(tokens);
}

char** split_string(const char* input, const char* delimiter, int* count) {
  // Create duplicate string
  char* temp_string = strdup(input);
  if (!temp_string) {
    log_error("Failed to allocate memory: %s", strerror(errno));
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
    log_error("Failed to allocate memory: %s", strerror(errno));
    exit(1);
  }

  temp_string = strdup(input);
  if (!temp_string) {
    log_error("Failed to allocate memory: %s", strerror(errno));
    free_tokens(tokens, num_tokens);
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

void setup_container(Arguments* args_ptr) {
  // Pids cgroup is updated on the host
  if (args_ptr->max_processes != NULL) set_pids_cgroup(args_ptr->max_processes);

  if (unshare(CLONE_NEWNS) == -1) {
    log_error("unshare failed: %s", strerror(errno));
    exit(1);
  }

  if (chdir("/home") == -1) {
    log_error("chdir failed: %s", strerror(errno));
    exit(1);
  }

  if (sethostname("container", sizeof("container")) == -1) {
    log_error("sethostname failed: %s", strerror(errno));
    exit(1);
  }

  if (chroot(args_ptr->filesystem_path) == -1) {
    log_error("chroot failed: %s", strerror(errno));
    exit(1);
  }

  if (chdir("/") == -1) {
    log_error("chdir failed: %s", strerror(errno));
    exit(1);
  }

  if (mount("proc", "proc", "proc", 0, NULL) == -1) {
    log_error("proc mount failed: %s", strerror(errno));
    exit(1);
  }
}

void run_container(Arguments* args_ptr) {
  int num_tokens = 0;
  char** tokens = split_string(args_ptr->args[1], " ", &num_tokens);

  if (execv(tokens[0], tokens) == -1) {
    log_error("(Child) execv failed: %s", strerror(errno));
    free_tokens(tokens, num_tokens);
    exit(1);
  }

  free_tokens(tokens, num_tokens);

  if (umount("proc") == -1) {
    log_error("unmount failed: %s", strerror(errno));
    exit(1);
  }

  return;
}

int run(Arguments* args_ptr) {
  struct clone_args cl_args = {0};
  cl_args.flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWCGROUP;
  cl_args.exit_signal = SIGCHLD;

  pid_t pid = syscall(SYS_clone3, &cl_args, sizeof(struct clone_args));

  if (pid == 0) {
    log_info("(Child) Starting.... %d", getpid());

    setup_container(args_ptr);
    log_info("(Child) Container initialized....");

    run_container(args_ptr);
    log_info("(Child) PID %d finished executing command. Exiting...", getpid());

    exit(0);
  } else if (pid > 0) {
    log_info("(Parent) Waiting.... %d", getpid());

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      log_info("(Parent) Child exited with status %d", WEXITSTATUS(status));
    }
    return 0;
  } else {
    // Negative pid (-1) means child was not created
    log_error("(Parent) Could not create child : %s. Are you root?",
              strerror(errno));
    return -1;
  }
}
