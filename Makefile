CC := gcc
CCARGS := -O3
CCOBJECTS := main.o args.o container.o

.SILENT: runix ${CCOBJECTS} clean 

runix: ${CCOBJECTS}
	${CC} ${CCARGS} ${CCOBJECTS} -o runix

main.o: main.c
	${CC} ${CCARGS} -c main.c

argc.o: args.c argc.h
	${CC} ${CCARGS} -c args.c

container.o: container.c container.h
	${CC} ${CCARGS} -c container.c
 
run: runix
	./runix run "/bin/bash"

clean:
	rm -rf runix ${CCOBJECTS}