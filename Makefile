.SILENT: runix main.o args.o clean

CC := gcc
CCARGS := -O3

runix: main.o args.o
	${CC} ${CCARGS} main.o args.o -o runix

main.o: main.c
	${CC} ${CCARGS} -c main.c

argc.o: args.c argc.h
	${CC} ${CCARGS} -c args.c

run: runix
	./runix run "/bin/bash"

clean:
	rm -rf runix args.o main.o