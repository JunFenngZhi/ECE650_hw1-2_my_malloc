These test programs are sample correctness test cases for the thread
safe malloc implementation. Note that these tests do not 
*guarantee* that your code is completely correct. If your code
does pass these tests, it is likely at least in pretty good shape.

When executed, the tests will either print (on test success):
No overlapping allocated regions found!
Test passed

Or on test failure, something like:
Found 2 overlapping allocated regions.
Region 1 bounds: start=ca0010, end=ca0090, size=128B, idx=0
Region 2 bounds: start=ca0010, end=ca01b0, size=416B, idx=30
Test failed

In the fail output, you can see here that malloc returned the same
starting address for two different malloc calls.

These programs work by creating N threads (using pthreads). Each
thread reaches a barrier at the start of its execution function
to give all threads time to get started. Then each thread loops
and mallocs / frees memory regions of random sizes, and then exits.

The programs check correctness by then looking at the address 
ranges of all malloc'ed regions to make sure that they are all
completely distinct (test pass).  If any regions overlap, then
the test fail message is printed.

There are two #define values in the c code that you may change
to easily change the number of threads or number of items each
thread will malloc:
#define NUM_THREADS  4
#define NUM_ITEMS    10

The test program "thread_test_measurement.c" will be your experimental
measurement test case for this assignment.  This test self-reports
the run-time and the total size of the data segment that is created
during the test. You may use this test to evaluate the differences
in performance and allocation efficiency between your lock-based and
non-locking versions of the thread-safe malloc library.  In this
test, multiple threads are created, which all perform malloc 
operations of various sizes. Only some of the threads (the threads
with an even ID) occasionally free allocations from all the threads.

To compile this program, you may work with the provided Makefile.
There is are 2 variable that you will need to edit:

1) WDIR should point to the directory with your my_malloc.* code
and compiled library (libmymalloc.so).

2) MALLOC_VERSION should either be set to "LOCK_VERSION" or 
"NOLOCK_VERSION" such that the test invokes the desired version
of your thread-safe malloc functions.


