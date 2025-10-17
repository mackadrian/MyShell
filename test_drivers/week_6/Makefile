# ----------------------
# Main shell target
# ----------------------
mysh: mysh.o mystring.o myheap.o runjob.o getjob.o errors.o signal.o builtin.o
	gcc mysh.o mystring.o myheap.o runjob.o getjob.o errors.o signal.o builtin.o -o mysh

# ----------------------
# Test driver for get_job
# ----------------------
test_getjob: test_getjob.o mystring.o myheap.o getjob.o errors.o signal.o
	gcc test_getjob.o mystring.o myheap.o getjob.o errors.o signal.o -o test_getjob

test_runjob: test_runjob.o mystring.o myheap.o runjob.o errors.o signal.o
	gcc test_runjob.o mystring.o myheap.o runjob.o errors.o signal.o -o test_runjob

# ----------------------
# Object files
# ----------------------
mysh.o: mysh.c mysh.h mystring.h jobs.h myheap.h signal.h
	gcc -c mysh.c

mystring.o: mystring.c mystring.h
	gcc -c mystring.c

myheap.o: myheap.c myheap.h
	gcc -c myheap.c

runjob.o: runjob.c jobs.h runjob.h errors.h
	gcc -c runjob.c

getjob.o: getjob.c jobs.h getjob.h errors.h signal.h
	gcc -c getjob.c

errors.o: errors.c errors.h
	gcc -c errors.c

signal.o: signal.c signal.h
	gcc -c signal.c

builtin.o: builtin.c builtin.h
	gcc -c builtin.c

# ----------------------
# Test driver object
# ----------------------
test_getjob.o: test_getjob.c jobs.h getjob.h mystring.h myheap.h errors.h
	gcc -c test_getjob.c

test_runjob.o: test_runjob.c jobs.h runjob.h mystring.h myheap.h errors.h
	gcc -c test_runjob.c

# ----------------------
# Clean
# ----------------------
clean:
	/usr/bin/rm -f *.o *~ mysh test_getjob test_runjob

# ----------------------
# Build everything
# ----------------------
all: mysh test_getjob test_runjob
