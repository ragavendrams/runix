#include "command_parser.h"
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <linux/sched.h>
#include <sched.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

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

void validate_arguments(int argc, char **argv) {
  if (argc != 3) {
    std::cout
        << "Error: Invalid number of arguments to application. Usage: "
           "./linux-container run '<linux command to run inside container>'."
        << std::endl;
    exit(1);
  }

  std::string mode = argv[1];
  if (mode != std::string("run")) {
    std::cout << "Error: Unsupported argument passed to container. Only 'run' "
                 "is supported"
              << std::endl;
    exit(2);
  }
}

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
  const char *cgroup_name = "linco";
  char path[100];

  sprintf(path, "%s/%s/%s", base_path, resource_folder, cgroup_name);
  if (mkdir(path, 0755) == -1) {
    perror("mkdir failed");
    exit(1);
  }

  write_resource(path, "pids.max", "20");

  write_resource(path, "notify_on_release", "1");

  char pid_str[10];
  sprintf(pid_str, "%d", getpid());
  write_resource(path, "cgroup.procs", pid_str);
}

void run_container(const std::vector<const char *> &args) {

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
    perror("mount failed");
    exit(1);
  }

  int fd = open("/dev/tty", O_RDWR | O_NONBLOCK);

  if (execv(args[0], const_cast<char *const *>(args.data())) == -1) {
    perror("(Child) execv failed");
    exit(1);
  }

  std::cout << "(Child) PID " + std::to_string((long)getpid()) +
                   " finished executing command."
            << std::endl;
  close(fd);

  if (umount("proc") == -1) {
    perror("unmount failed");
    exit(1);
  }

  exit(0);
}

void run(const std::vector<const char *> &args) {

  struct clone_args clargs = {0};
  clargs.flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWCGROUP;
  clargs.exit_signal = SIGCHLD;

  pid_t pid = syscall(SYS_clone3, &clargs, sizeof(struct clone_args));

  if (pid == 0) {
    std::cout << "(Child) Starting...." << getpid() << std::endl;
    run_container(args);

  } else if (pid > 0) {
    std::cout << "(Parent) Waiting...." << getpid() << std::endl;
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

int main(int argc, char *argv[]) {

  validate_arguments(argc, argv);

  CommandParser parser;
  auto args = parser.parse(argv[2]);

  run(args);

  return 0;
}