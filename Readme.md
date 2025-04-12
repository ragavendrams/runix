## Runix - A minimal Linux container runtime in C

This repository implements a minimal low-level linux container runtime from scratch in C. Goal here was to learn how containers work. Industry standards for low-level container runtimes would be runC, containerd, CRI-O etc.  
This implementation uses a combination of features such as chroot, cgroups(v1) and namespaces to regulate the container. The container created has the following features:
- Separate filesystem (using chroot jail)
- Separate Process tree and hostname (using namespaces) 
- Limited resource usage, e.g a pids limit (using cgroups v1)

## Setup 
- Download a POSIX compliant root filesystem. Eg. To download a minimal root filesystem for Ubuntu 22.04,  
```console
root@host:~/repos/runix$ wget https://cdimage.ubuntu.com/ubuntu-base/releases/22.04/release/ubuntu-base-22.04.5-base-amd64.tar.gz
root@host:~/repos/runix$ mkdir fs/ubuntu-fs && tar -xzf ubuntu-base-22.04.5-base-amd64.tar.gz -C fs/ubuntu-fs
``` 
- Get source files
```console 
root@host:~/repos/runix$ git clone --recurse-submodules https://github.com/ragavendrams/runix.git
``` 
- Install CMake and GCC
```console 
root@host:~/repos/runix$ sudo apt update && sudo apt install cmake gcc-11

# Ensure everything is installed
root@host:~/repos/runix$ cmake --version
root@host:~/repos/runix$ gcc --version
``` 

## Running the container

- Build the application and run it (ensure to run in a prompt with elevated privileges)
```console 
root@host:~/repos/runix$ mkdir build-release && cd build-release && cmake .. -DCMAKE_BUILD_TYPE=Release 
root@host:~/repos/runix/build-release$ cmake --build . --target runix --parallel $(nproc)

# Start a bash shell (The shell needs to be available in the filesystem)
root@ubuntu2204:/home/raga/repos/runix/build-release# ./runix run -r ../fs/ubuntu-fs -p 20 -v "/bin/bash"
13:31:26 INFO  /home/raga/repos/runix/src/container.c:151: (Parent) Waiting.... 28755
13:31:26 INFO  /home/raga/repos/runix/src/container.c:141: (Child) Starting.... 1
11:31:26 INFO  /home/raga/repos/runix/src/container.c:144: (Child) Container initialized....
root@container:/# ls -l 
total 56
lrwxrwxrwx   1 1000 1000    7 Sep 11  2024 bin -> usr/bin
drwxr-xr-x   2 1000 1000 4096 Apr 18  2022 boot
drwxr-xr-x   3 1000 1000 4096 Apr 11 08:53 dev
drwxr-xr-x  31 1000 1000 4096 Sep 11  2024 etc
drwxr-xr-x   2 1000 1000 4096 Apr 18  2022 home
lrwxrwxrwx   1 1000 1000    7 Sep 11  2024 lib -> usr/lib
lrwxrwxrwx   1 1000 1000    9 Sep 11  2024 lib32 -> usr/lib32
lrwxrwxrwx   1 1000 1000    9 Sep 11  2024 lib64 -> usr/lib64
lrwxrwxrwx   1 1000 1000   10 Sep 11  2024 libx32 -> usr/libx32
drwxr-xr-x   2 1000 1000 4096 Sep 11  2024 media
drwxr-xr-x   2 1000 1000 4096 Sep 11  2024 mnt
drwxr-xr-x   2 1000 1000 4096 Sep 11  2024 opt
dr-xr-xr-x 222 root root    0 Apr 12 11:31 proc
drwx------   2 1000 1000 4096 Apr 11 14:09 root
drwxr-xr-x   4 1000 1000 4096 Sep 11  2024 run
lrwxrwxrwx   1 1000 1000    8 Sep 11  2024 sbin -> usr/sbin
drwxr-xr-x   2 1000 1000 4096 Sep 11  2024 srv
drwxr-xr-x   2 1000 1000 4096 Apr 18  2022 sys
drwxr-xr-x   2 1000 1000 4096 Sep 11  2024 tmp
drwxr-xr-x  14 1000 1000 4096 Sep 11  2024 usr
drwxr-xr-x  11 1000 1000 4096 Sep 11  2024 var
root@container:/# ps -ax
  PID TTY      STAT   TIME COMMAND
    1 ?        S      0:00 /bin/bash
    5 ?        R+     0:00 ps -ax
root@container:/# echo "container"
container
root@container:/# exit
exit
13:32:18 INFO  /home/raga/repos/runix/src/container.c:156: (Parent) Child exited with status 0
root@ubuntu2204:/home/raga/repos/runix/build-release# 
``` 

## Limitations
- Doesn't support pull/push of images. For now, the filesystem has to be provided to the application. 
- Currently only supports limiting max processes allowed within the container. Other cgroup controllers could also be used in the future. 
- Container process is in the host's process group. Ideally the container process has its own process group and its own controlling terminal.
- User needs to be root to run the container. Rootless containers could be added in the future by using user namespaces.  
