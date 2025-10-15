mysh: mysh.o mystring.o myheap.o runjob.o getjob.o errors.o
	gcc mysh.o mystring.o myheap.o runjob.o getjob.o errors.o -o mysh

mysh.o: mysh.c mysh.h mystring.h jobs.h myheap.h
	gcc -c mysh.c

mystring.o: mystring.c mystring.h
	gcc -c mystring.c

myheap.o: myheap.c myheap.h
	gcc -c myheap.c

runjob.o: runjob.c jobs.h runjob.h errors.h
	gcc -c runjob.c

getjob.o: getjob.c jobs.h getjob.h errors.h
	gcc -c getjob.c

errors.o: errors.c errors.h
	gcc -c errors.c

clean:
	/usr/bin/rm -f *.o mysh

all: clean mysh
