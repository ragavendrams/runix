.SILENT: linux-container container.o command_parser.o clean

CC := g++
CCARGS := --std=c++17 -O3

linux-container: container.o command_parser.o
	${CC} ${CCARGS} container.o command_parser.o -o linux-container

container.o: container.cpp command_parser.h
	${CC} ${CCARGS} -c container.cpp

command_parser.o: command_parser.cpp command_parser.h
	${CC} ${CCARGS} -c command_parser.cpp

run: linux-container
	./linux-container run "/bin/bash"

clean:
	rm -rf command_parser.o container.o linux-container