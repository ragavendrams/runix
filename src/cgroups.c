#include "cgroups.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"

#define MAX_CGROUP_PATH_LENGTH 256

void write_resource(const char *base_path, const char *resource_name,
                    const char *resource_value) {
  int path_len = strlen(resource_name) + strlen(base_path);
  if (path_len > MAX_CGROUP_PATH_LENGTH) {
    log_error("Error: Path length of cgroup resource exceeded 256 characters.");
  }

  char *full_path = malloc(path_len + 1);
  sprintf(full_path, "%s/%s", base_path, resource_name);

  FILE *file = fopen(full_path, "w");
  if (file) {
    fputs(resource_value, file);
    fclose(file);
    free(full_path);
  } else {
    log_error("Unable to write resource to disk.");
    free(full_path);
    exit(1);
  }
}

bool is_valid_pids_max(const char *pids_limit) {
  // Check if value is 'max' which is considered valid
  if (strcmp(pids_limit, "max") == 0) {
    return true;
  }

  // Check that every char is a digit
  for (size_t i = 0; i < strlen(pids_limit); i++) {
    if (!isdigit(pids_limit[i])) {
      return false;
    }
  }

  // Check that the value is positive
  long input = strtol(pids_limit, NULL, 10);
  if (input < 0) {
    return false;
  }

  return true;
}

void set_pids_cgroup(const char *pids_limit) {
  if (!is_valid_pids_max(pids_limit)) {
    log_error(
        "Error: Invalid value passed to --pids-limit / -p. Value must be "
        "'max' or an unsigned integer string.");
    exit(1);
  }

  // Supports only Cgroups v1 for now
  const char *path = "/sys/fs/cgroup/pids/runix";
  if (mkdir(path, 0755) == -1) {
    if (errno != EEXIST) {
      log_error("mkdir failed: %s", strerror(errno));
      exit(1);
    }
  }

  write_resource(path, "pids.max", pids_limit);
  write_resource(path, "notify_on_release", "1");

  char pid_str[6];  // PIDS_MAX is 32768 in POSIX systems -> 5 characters + 1
                    // (null)
  snprintf(pid_str, sizeof(pid_str), "%d", getpid());
  write_resource(path, "cgroup.procs", pid_str);
}
