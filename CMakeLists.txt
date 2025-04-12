cmake_minimum_required(VERSION 3.22)
project(runix VERSION 1.0.0)

add_executable(runix src/main.c 
					 src/arguments.c
					 src/cgroups.c	
					 src/container.c
					 lib/log/src/log.c)
target_include_directories(runix PUBLIC include lib/log/src)
target_compile_definitions(runix PRIVATE LOG_USE_COLOR)
