#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "my_malloc.h"

#ifdef LOCK_VERSION
#define MALLOC(sz) ts_malloc_lock(sz)
#define FREE(p)    ts_free_lock(p)
#endif
#ifdef NOLOCK_VERSION
#define MALLOC(sz) ts_malloc_nolock(sz)
#define FREE(p)    ts_free_nolock(p)
#endif

#define NUM_THREADS  4
#define NUM_ITEMS    20000

double calc_time(struct timespec start, struct timespec end) {
  double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
  double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;

  if (end_sec < start_sec) {
    return 0;
  } else {
    return end_sec - start_sec;
  }
};


pthread_t threads[NUM_THREADS];
int       thread_id[NUM_THREADS];

pthread_barrier_t barrier;
pthread_mutex_t   my_mutex = PTHREAD_MUTEX_INITIALIZER;

struct malloc_list {
  size_t bytes;
  int *address;
  int free;
};
typedef struct malloc_list malloc_list_t;

malloc_list_t malloc_items[NUM_THREADS * NUM_ITEMS];


void do_allocate(int thread_id) {
  int i, index;
  int do_free;
  //Rotate the counter so that each thread will free addresses
  //that were malloc'ed by another thread.
  int counter = ((thread_id+1)%NUM_THREADS) * NUM_ITEMS;
  int thread_start_index = thread_id * NUM_ITEMS;

  //Let all threads get up and running
  //Want the concurrent malloc calls to be as high as possible
  pthread_barrier_wait(&barrier); 

  for (i=0; i < NUM_ITEMS; i++) {
    index = i + thread_start_index;
    malloc_items[index].address = (int *)MALLOC(malloc_items[index].bytes);
    malloc_items[index].free = 0;

    if ((thread_id % 2) == 0) {
      if ((i % 4) == 0) { //Occasionally free some items
	pthread_mutex_lock(&my_mutex);
	if (malloc_items[counter].free == 0) {
	  malloc_items[counter].free = 1;
	  do_free = 1;
	} else {
	  do_free = 0;
	} //else
	pthread_mutex_unlock(&my_mutex);
	if (do_free == 1) {
	  FREE(malloc_items[counter].address);
	  counter++;
	} //if
      } //if
    }
  } //for i

  pthread_barrier_wait(&barrier);
}


void *allocate(void *arg) {
  int id = *((int *) arg);
  do_allocate(id);
  return NULL;
} 


int main(int argc, char *argv[])
{
  int i, j;
  struct timespec start_time, end_time;
  void *start_segment_addr, *end_segment_addr;

  srand(0);

  const unsigned chunk_size = 32;
  const unsigned min_chunks = 4;
  const unsigned max_chunks = 32;
  //const unsigned max_chunks = 16384;
  for (i=0; i < NUM_THREADS*NUM_ITEMS; i++) {
    unsigned num_chunks = (rand() % (max_chunks - min_chunks + 1)) + min_chunks;
    malloc_items[i].bytes = num_chunks * chunk_size;
    malloc_items[i].free = 1;
  } //for i

  pthread_barrier_init(&barrier, NULL, NUM_THREADS);

  start_segment_addr = sbrk(0);
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  for (i=0; i < NUM_THREADS; i++) {
    thread_id[i] = i;
    pthread_create(&threads[i], NULL, allocate, (void *)(&thread_id[i]));
  } //for i
  for (i=0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  } //for i
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  end_segment_addr = sbrk(0);

  //Check for correctness!

  int *start, *end, *tgt_start, *tgt_end;
  int fail = 0;
  for (i=0; i < NUM_THREADS * NUM_ITEMS; i++) {
    if (malloc_items[i].free == 1) continue;
    start = malloc_items[i].address;
    end   = start + (malloc_items[i].bytes / sizeof(int));

    for (j=0; j < NUM_THREADS * NUM_ITEMS; j++) {
      if (malloc_items[j].free == 1) continue;
      if (i == j) continue;
      tgt_start = malloc_items[j].address;
      tgt_end   = tgt_start + (malloc_items[j].bytes / sizeof(int));
      if (((start >= tgt_start) && (start <= tgt_end)) ||
	  ((end >= tgt_start) && (end <= tgt_end))) {
	fail = 1;
	break;
      } //if
    }

    if (fail == 1) break;
  } //for i

  if (fail == 0) {
    printf("No overlapping allocated regions found!\n");
    printf("Test passed\n");
  } else {
    printf("Found 2 overlapping allocated regions.\n");
    printf("Region 1 bounds: start=%llx, end=%llx, size=%dB, idx=%d\n", start, end, malloc_items[i].bytes, i);
    printf("Region 2 bounds: start=%llx, end=%llx, size=%dB, idx=%d\n", tgt_start, tgt_end, malloc_items[j].bytes, j);
    printf("Test failed\n");
  } //else

  for (i=0; i < NUM_THREADS * NUM_ITEMS; i++) {
    if (malloc_items[i].free == 0) {
      FREE(malloc_items[i].address);
    } //if
  } //for i

  double elapsed_ns = calc_time(start_time, end_time);
  printf("Execution Time = %f seconds\n", elapsed_ns / 1e9);
  printf("Data Segment Size = %lu bytes\n", (unsigned long)(end_segment_addr - start_segment_addr));

  return 0;
}
