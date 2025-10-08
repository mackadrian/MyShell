testsh: testsh.o mystring.o myheap.o
	gcc testsh.o mystring.o myheap.o -o testsh

testsh.o: testsh.c mysh.h mystring.h jobs.h myheap.h
	gcc -c testsh.c

mystring.o: mystring.c mystring.h
	gcc -c mystring.c

myheap.o: myheap.c myheap.h
	gcc -c myheap.c

clean:
	/usr/bin/rm -f *.o mysh

all: clean mysh
