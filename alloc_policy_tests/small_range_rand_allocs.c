#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "my_malloc.h"

#define NUM_ITERS 100 //必须是偶数
#define NUM_ITEMS 10000
#define NUM_STEP 50

#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p) ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p) bf_free(p)
#endif

double calc_time(struct timespec start, struct timespec end)
{
  double start_sec = (double)start.tv_sec * 1000000000.0 + (double)start.tv_nsec;
  double end_sec = (double)end.tv_sec * 1000000000.0 + (double)end.tv_nsec;

  if (end_sec < start_sec)
  {
    return 0;
  }
  else
  {
    return end_sec - start_sec;
  }
};

struct malloc_list
{
  size_t bytes;
  int *address;
};
typedef struct malloc_list malloc_list_t;

malloc_list_t malloc_items[2][NUM_ITEMS];

unsigned free_list[NUM_ITEMS];

int main(int argc, char *argv[])
{
  int i, j, k;
  unsigned tmp;
  unsigned long data_segment_size;
  unsigned long data_segment_free_space;
  struct timespec start_time, end_time;

  srand(0);

  const unsigned chunk_size = 32;
  const unsigned min_chunks = 4;
  const unsigned max_chunks = 16;
  for (i = 0; i < NUM_ITEMS; i++)
  {
    malloc_items[0][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
    malloc_items[1][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
    free_list[i] = i;
  } //for i

  i = NUM_ITEMS;
  while (i > 1)
  {
    i--;
    j = rand() % i;
    tmp = free_list[i];
    free_list[i] = free_list[j];
    free_list[j] = tmp;
  } //while

  //申请第一组空间
  for (i = 0; i < NUM_ITEMS; i++)
  {
    malloc_items[0][i].address = (int *)MALLOC(malloc_items[0][i].bytes);
    //printf("address:%d, size:%d\n",malloc_items[0][i].address,malloc_items[0][i].bytes);
    //printf("----------------------------\n");
  }

  //Start Time
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  for (i = 0; i < NUM_ITERS; i++)
  {
    //printf("-----------------------ITERATION %d-----------------------\n", i + 1);
    //getListLength();
    unsigned malloc_set = i % 2;
    for (j = 0; j < NUM_ITEMS; j += 50) //每轮循环随机free NUM_STEP个，再随机malloc NUM_STEP个
    {
      for (k = 0; k < NUM_STEP; k++)
      { 
        unsigned item_to_free = free_list[j + k];
        FREE(malloc_items[malloc_set][item_to_free].address);
      }
      //printf("After free blocks:\n");
      //printLinkedList(); // Test breakpoint 2
      //assert(cycleDetect() == 0);
      //getListLength();

      for (k = 0; k < NUM_STEP; k++)
      {
        malloc_items[1 - malloc_set][j + k].address = (int *)MALLOC(malloc_items[1 - malloc_set][j + k].bytes);
      }
      //printf("After malloc blocks:");
      //printLinkedList(); // Test breakpoint 3
      //assert(cycleDetect() == 0);
      //getListLength();
    }
  }

  //Stop Time
  clock_gettime(CLOCK_MONOTONIC, &end_time);

  data_segment_size = get_data_segment_size();
  data_segment_free_space = get_data_segment_free_space_size();
  printf("data_segment_size = %lu, data_segment_free_space = %lu\n", data_segment_size, data_segment_free_space);

  double elapsed_ns = calc_time(start_time, end_time);
  printf("Execution Time = %f seconds\n", elapsed_ns / 1e9);
  printf("Fragmentation  = %f\n", (float)data_segment_free_space / (float)data_segment_size);

  for (i = 0; i < NUM_ITEMS; i++)
  {
    FREE(malloc_items[0][i].address);
  } //for i

  return 0;
}
