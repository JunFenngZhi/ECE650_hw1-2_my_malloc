#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "my_malloc.h"

#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p) ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p) bf_free(p)
#endif

#define NUM_ITEMS 20
#define DIVISION 2

int main(int argc, char *argv[])
{
  int *array[NUM_ITEMS];
  int freeMark[NUM_ITEMS] = {0}; //标记对应block有没有被free

  //create random array
  int random[NUM_ITEMS / DIVISION];
  srand((unsigned)time(NULL));
  for (int i = 0; i < NUM_ITEMS / DIVISION; i++)
  {
    int n = rand() % NUM_ITEMS;
    int repeat = 0;
    for (int k = 0; k < i; k++) //判断是否和前面重复
    {
      if (n == random[k])
      {
        repeat = 1;
        break;
      }
    }
    if (repeat == 1)
    {
      i--;
      continue;
    }
    else
    {
      random[i] = n;
      printf("random[i]:%d   ", random[i]);
    }
  }
  printf("\n");

  printf("//------------ROUND 1-----------------------\n");
  //malloc blocks
  for (int i = 0; i < NUM_ITEMS; i++)
  {
    array[i] = (int *)MALLOC(sizeof(int));
    array[i][0] = i;
  }

  //randomly free some blocks
  for (int i = 0; i < NUM_ITEMS / DIVISION; i++)
  {
    FREE(array[random[i]]);
    freeMark[random[i]] = 1; //被free
  }

  //check reamain block value
  for (int i = 0; i < NUM_ITEMS; i++)
  {
    if (freeMark[i] == 0)
      assert(array[i][0] == i);
  }
  printLinkedList();

  printf("//------------ROUND 2-----------------------\n");
  //malloc blocks from previous free blocks
  for (int i = 0; i < NUM_ITEMS/DIVISION; i++)
  {
    printf("malloc block[%d]\n", random[i]);
    array[random[i]] = (int *)MALLOC(sizeof(int));
    array[random[i]][0] = random[i];
    freeMark[random[i]] = 0;
    printLinkedList();
  }
  printLinkedList(); //这个地方应该list为空

  //randomly free some blocks again
  for (int i = 0; i < NUM_ITEMS / DIVISION; i++)
  {
    FREE(array[random[i]]);
    freeMark[random[i]] = 1; //被free
  }
  printLinkedList();

  //check reamain block value
  for (int i = 0; i < NUM_ITEMS; i++)
  {
    if (freeMark[i] == 0)
      assert(array[i][0] == i);
  }

  printf("//------------Finally-----------------------\n");
  //free reamain blocks
  for (int i = 0; i < NUM_ITEMS; i++)
  {
    if (freeMark[i] == 0)
      FREE(array[i]);
  }

  printLinkedList();

  return 0;
}

/*
void origin_test()
{
  int i;
  int size;
  int sum = 0;
  int expected_sum = 0;
  int *array[NUM_ITEMS];

  size = 4;
  expected_sum += size * size;
  array[0] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[0][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[0][i];
  } //for i

  size = 16;
  expected_sum += size * size;
  array[1] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[1][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[1][i];
  } //for i

  size = 8;
  expected_sum += size * size;
  array[2] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[2][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[2][i];
  } //for i

  size = 32;
  expected_sum += size * size;
  array[3] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[3][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[3][i];
  } //for i

  FREE(array[0]);
  FREE(array[2]);

  size = 7;
  expected_sum += size * size;
  array[4] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[4][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[4][i];
  } //for i

  size = 256;
  expected_sum += size * size;
  array[5] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[5][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[5][i];
  } //for i

  FREE(array[5]);
  FREE(array[1]);
  FREE(array[3]);

  size = 23;
  expected_sum += size * size;
  array[6] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[6][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[6][i];
  } //for i

  size = 4;
  expected_sum += size * size;
  array[7] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[7][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[7][i];
  } //for i

  FREE(array[4]);

  size = 10;
  expected_sum += size * size;
  array[8] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[8][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[8][i];
  } //for i

  size = 32;
  expected_sum += size * size;
  array[9] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[9][i] = size;
  } //for i
  for (i = 0; i < size; i++)
  {
    sum += array[9][i];
  } //for i

  FREE(array[6]);
  FREE(array[7]);
  FREE(array[8]);
  FREE(array[9]);

  if (sum == expected_sum)
  {
    printf("Calculated expected value of %d\n", sum);
    printf("Test passed\n");
  }
  else
  {
    printf("Expected sum=%d but calculated %d\n", expected_sum, sum);
    printf("Test failed\n");
  } //else

  return 0;
}
*/