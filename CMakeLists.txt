cmake_minimum_required(VERSION 3.22)
project(runix VERSION 1.0.0)

add_executable(runix src/main.c 
					 src/arguments.c
					 src/cgroups.c	
					 src/container.c)
target_include_directories(runix PUBLIC include)

