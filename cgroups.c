#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

#include "cgroups.h"

#define MAX_CGROUP_PATH_LENGTH 256

void write_resource(const char *base_path, const char *resource_name,
	const char *resource_value) {

	int path_len = strlen(resource_name) + strlen(base_path);
	if(path_len > MAX_CGROUP_PATH_LENGTH){
		fprintf(stderr, "Error: Path length of cgroup resource exceeded 256 characters.");
	}

	char* full_path = malloc(path_len + 1);
	sprintf(full_path, "%s/%s", base_path, resource_name);

	FILE *file = fopen(full_path, "w");
	if (file) {
		fputs(resource_value, file);
		fclose(file);
		free(full_path);
	} else {
		printf("Unable to write resource to disk.");
		free(full_path);
		exit(1);
	}
}

bool is_valid_pids_max(const char *pids_limit){
	// Check if value is 'max' which is considered valid
	if(strcmp(pids_limit, "max") == 0){
		return true;
	}

	// Check that every char is a digit
	for(size_t i = 0; i < strlen(pids_limit); i++){
		if(!isdigit(pids_limit[i])){
			return false;
		}
	}

	// Check that the value is positive
	long input = strtol(pids_limit, NULL, 10);
	if(input < 0){
		return false;
	}

	return true;
}

void set_pids_cgroup(const char *pids_limit) {

	if(!is_valid_pids_max(pids_limit)){
		fprintf(stderr, "Error: Invalid value passed to --pids-limit / -p. Value must be 'max' or an unsigned integer string.");
		exit(1);
	}

	const char *path = "/sys/fs/cgroup/runix";
	if (mkdir(path, 0755) == -1) {
		if (errno != EEXIST) {
		perror("mkdir failed");
		exit(1);
		}
	}

	write_resource(path, "pids.max", pids_limit);
	write_resource(path, "notify_on_release", "1");

	char pid_str[6]; // PIDS_MAX is 32768 in POSIX systems -> 5 characters + 1 (null) 
	snprintf(pid_str, sizeof(pid_str), "%d", getpid());
	write_resource(path, "cgroup.procs", pid_str);
}
