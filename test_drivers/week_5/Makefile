testsh: testsh.o mystring.o myheap.o runjob.o getjob.o errors.o
	gcc testsh.o mystring.o myheap.o runjob.o getjob.o errors.o -o testsh

testsh.o: testsh.c mysh.h mystring.h jobs.h myheap.h
	gcc -c testsh.c

mystring.o: mystring.c mystring.h
	gcc -c mystring.c

myheap.o: myheap.c myheap.h
	gcc -c myheap.c

runjob.o: runjob.c jobs.h runjob.h
	gcc -c runjob.c

getjob.o: getjob.c jobs.h getjob.h 
	gcc -c getjob.c

errors.o: errors.c errors.h
	gcc -c errors.c

clean:
	/usr/bin/rm -f *.o mysh

all: clean mysh
