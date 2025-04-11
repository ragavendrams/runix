#ifndef RUNIX_INCLUDE_CGROUPS_H
#define RUNIX_INCLUDE_CGROUPS_H

void write_resource(const char *base_path, const char *resource_name,
                    const char *resource_value);

void set_pids_cgroup(const char *pids_limit);

#endif