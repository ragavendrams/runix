## Linux container runtime in C

This repository implements a linux container runtime from scratch in C. It uses a combination of Linux features such as chroot, cgroups(v1) and namespaces. The corresponding syscalls/wrappers are called from C. The container runtime has the following features:
- Bidirectional process isolation with host
- Separate filesystem
- Can also impose limitations on resource usage (e.g max processes)
- Separate hostname 


## Setup 
- Download a POSIX compliant rootfs filesystem. Eg. To download the alpine rootfs,  
```
	wget https://dl-cdn.alpinelinux.org/alpine/v3.21/releases/x86_64/alpine-minirootfs-3.21.3-x86_64.tar.gz
	tar -xzf alpine-minirootfs-3.21.3-x86_64.tar.gz -C alpine-fs
``` 
- Get source files
``` 
	git clone https://github.com/ragavendrams/linux-container-from-scratch.git
``` 
- Install Make and GCC
``` 
	sudo apt update && sudo apt install build-essential
	
	# Ensure everything is installed
	make --version
	gcc --version
``` 

## Running the container

- Build the application and run it (ensure to run in a prompt with elevated privileges)
``` 
	# Build the application
	make 
	
	# Start a bash shell (The shell needs to be available in the rootfs)
	./runix run "/bin/bash"

	# Install packages
	./runix run "apt install wget"
 
``` 
- If everything goes well, you should see a shell with the prompt `root@container:/#`

- Type exit to end the process and return to host.  

## Limitations
- Doesn't support pulling an image from docker-hub or other registries and starting the container using it. For now, the rootfs has to be provided to the application. 
- No layers, caching and use of union filesystem, like in docker.
- No way to load or save the container. Right now, if you reuse the same rootfs folder, the state of the container is persistent. Ideally, you get a new filesystem every time a new container is created. 
- Only a few resources are limited - Currently supports limiting max processes.  
