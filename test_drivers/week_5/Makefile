testsh: testsh.o mystring.o myheap.o runjob.o getjob.o
	gcc testsh.o mystring.o myheap.o runjob.o getjob.o -o testsh

testsh.o: testsh.c mysh.h mystring.h jobs.h myheap.h
	gcc -c testsh.c

mystring.o: mystring.c mystring.h
	gcc -c mystring.c

myheap.o: myheap.c myheap.h
	gcc -c myheap.c

runjob.o: jobs.h runjob.h
	gcc -c runjob.c

getjob.o: jobs.h getjob.h
	gcc -c getjob.c

clean:
	/usr/bin/rm -f *.o mysh

all: clean mysh
