CC = gcc
CFLAGS = -c

# ----------------------
# Main shell target
# ----------------------
mysh: mysh.o mystring.o myheap.o runjob.o getjob.o errors.o signal.o
	$(CC) mysh.o mystring.o myheap.o runjob.o getjob.o errors.o signal.o -o mysh

# ----------------------
# Test driver for get_job
# ----------------------
test_getjob: test_getjob.o mystring.o myheap.o getjob.o errors.o signal.o
	$(CC) test_getjob.o mystring.o myheap.o getjob.o errors.o signal.o -o test_getjob

test_runjob: test_runjob.o mystring.o myheap.o runjob.o errors.o signal.o
	$(CC) test_runjob.o mystring.o myheap.o runjob.o errors.o signal.o -o test_runjob

# ----------------------
# Object files
# ----------------------
mysh.o: mysh.c mysh.h mystring.h jobs.h myheap.h signal.h
	$(CC) $(CFLAGS) mysh.c

mystring.o: mystring.c mystring.h
	$(CC) $(CFLAGS) mystring.c

myheap.o: myheap.c myheap.h
	$(CC) $(CFLAGS) myheap.c

runjob.o: runjob.c jobs.h runjob.h errors.h
	$(CC) $(CFLAGS) runjob.c

getjob.o: getjob.c jobs.h getjob.h errors.h signal.h
	$(CC) $(CFLAGS) getjob.c

errors.o: errors.c errors.h
	$(CC) $(CFLAGS) errors.c

signal.o: signal.c signal.h
	$(CC) $(CFLAGS) signal.c

# ----------------------
# Test driver object
# ----------------------
test_getjob.o: test_getjob.c jobs.h getjob.h mystring.h myheap.h errors.h
	$(CC) $(CFLAGS) test_getjob.c

test_runjob.o: test_runjob.c jobs.h runjob.h mystring.h myheap.h errors.h
	$(CC) $(CFLAGS) test_runjob.c

# ----------------------
# Clean
# ----------------------
clean:
	/usr/bin/rm -f *.o mysh test_getjob test_runjob

# ----------------------
# Build everything
# ----------------------
all: mysh test_getjob test_runjob
