# ----------------------
# Main shell target
# ----------------------
mysh: mysh.o mystring.o myheap.o runjob.o getjob.o errors.o signal.o builtin.o
	gcc mysh.o mystring.o myheap.o runjob.o getjob.o errors.o signal.o builtin.o -o mysh

# ----------------------
# Test drivers (executables in test_drivers/)
# ----------------------
test_drivers/test_getjob: test_drivers/test_getjob.o mystring.o myheap.o getjob.o errors.o
	gcc test_drivers/test_getjob.o mystring.o myheap.o getjob.o errors.o -o test_drivers/test_getjob

test_drivers/test_runjob: test_drivers/test_runjob.o mystring.o myheap.o runjob.o errors.o signal.o
	gcc test_drivers/test_runjob.o mystring.o myheap.o runjob.o errors.o signal.o -o test_drivers/test_runjob

# ----------------------
# Object files for main shell
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
# Test driver object files
# ----------------------
test_drivers/test_getjob.o: test_drivers/test_getjob.c jobs.h getjob.h mystring.h myheap.h errors.h signal.h
	gcc -I. -I.. -c test_drivers/test_getjob.c -o test_drivers/test_getjob.o

test_drivers/test_runjob.o: test_drivers/test_runjob.c jobs.h runjob.h mystring.h myheap.h errors.h signal.h
	gcc -I. -I.. -c test_drivers/test_runjob.c -o test_drivers/test_runjob.o

# ----------------------
# Clean
# ----------------------
clean:
	/usr/bin/rm -f *.o *~ mysh \
	test_drivers/test_getjob \
	test_drivers/test_runjob \
	test_drivers/*.o

# ----------------------
# Build everything
# ----------------------
all: mysh test_drivers/test_getjob test_drivers/test_runjob
