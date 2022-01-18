#include "my_malloc.h"

#define SPILT_THRESHOLD 8

void *makeNewBlock(const size_t size);
void *unFreeOldBlock(metaInfo *ptr_m, const size_t size);
void insList(metaInfo *ptr_m);
metaInfo *mergeBlock(metaInfo *p1_m, metaInfo *p2_m);

size_t heapSize = 0;
metaInfo *freeList = NULL; //head of freeList

////////////////////First Fitting Policy///////////////////////////
void *ff_malloc(size_t size)
{
  void *ptr = NULL;

  //search freeList to allocation proper block(First fit policy)
  metaInfo *ptr_m = NULL;
  for (ptr_m = freeList; ptr_m != NULL; ptr_m = ptr_m->next)
  {
    if (ptr_m->size >= size)
      break;
  }

  if (ptr_m != NULL)
  { //find fitting block
    ptr = unFreeOldBlock(ptr_m, size);
  }
  else
  { //no fitting block
    ptr = makeNewBlock(size);
  }

  return ptr + sizeof(metaInfo);
}

void ff_free(void *ptr)
{
  if (ptr == NULL)
    return;

  metaInfo *ptr_m = ptr - sizeof(metaInfo);
  insList(ptr_m);

  //merge adjacent free block
  metaInfo *prev = ptr_m->prev;
  metaInfo *next = ptr_m->next;
  metaInfo *merge_ptr = mergeBlock(prev, ptr_m);
  mergeBlock(merge_ptr, next);

  return;
}

////////////////////Best Fitting Policy///////////////////////////
void *bf_malloc(size_t size)
{
  void *ptr = NULL;

  //search freeList to allocation proper block(Best fit policy)
  metaInfo *ptr_BF = NULL;
  size_t minDiff = __SIZE_MAX__;
  for (metaInfo *ptr_m = freeList; ptr_m != NULL; ptr_m = ptr_m->next)
  {
    if (ptr_m->size >= size && ptr_m->size - size < minDiff)
    {
      minDiff = ptr_m->size - size;
      ptr_BF = ptr_m;
    }
    if (minDiff == 0)
      break;
  }

  if (ptr_BF != NULL) //find fitting block
    ptr = unFreeOldBlock(ptr_BF, size);
  else //no fitting block
    ptr = makeNewBlock(size);

  return ptr + sizeof(metaInfo);
}

void bf_free(void *ptr)
{
  ff_free(ptr);
}

////////////////////////////Helper Functions//////////////////////////
metaInfo *mergeBlock(metaInfo *p1_m, metaInfo *p2_m) //p1_m points to the front block,p2_m points to the next block
{
  if (p1_m == NULL)
    return p2_m;
  if (p2_m == NULL)
    return p1_m;

  void *p1 = p1_m;
  void *p2 = p2_m;
  if (p2 == p1 + sizeof(metaInfo) + p1_m->size)
  { // two blocks is adjacent
    p1_m->size += (sizeof(metaInfo) + p2_m->size);
    p1_m->next = p2_m->next;
    if (p2_m->next != NULL)
    {
      p2_m->next->prev = p1_m;
    }
    p2_m->size = 0;
    p2_m->next = NULL;
    p2_m->prev = NULL;

    return p1_m;
  }
  return p2_m;
}

void insList(metaInfo *ptr_m)
{
  metaInfo **cur = &freeList;
  metaInfo *prev = NULL;

  //find insert position;
  while (*cur != NULL && ptr_m > *cur)
  {
    prev = *cur;
    cur = &((*cur)->next);
  }

  //insert
  ptr_m->next = *cur;
  (*cur) = ptr_m;
  ptr_m->prev = prev;
  if (ptr_m->next != NULL)
    ptr_m->next->prev = ptr_m;

  return;
}

void *makeNewBlock(const size_t size)
{
  size_t blockSize = size + sizeof(metaInfo); //new block size
  void *ptr = sbrk((intptr_t)blockSize);
  assert(ptr != NULL);

  metaInfo *ptr_m = (metaInfo *)ptr; // set new block's meta data
  ptr_m->size = size;
  ptr_m->next = NULL;
  ptr_m->prev = NULL;

  heapSize += blockSize;

  return ptr;
}

void *unFreeOldBlock(metaInfo *ptr_m, const size_t size)
{
  if (ptr_m->size > size + sizeof(metaInfo) + SPILT_THRESHOLD)
  { // remain and spilt
    void *remain_node = (void *)ptr_m + sizeof(metaInfo) + size;

    //reset the remain_node
    metaInfo *ptr_remain = remain_node;
    ptr_remain->size = ptr_m->size - size - sizeof(metaInfo);
    ptr_remain->prev = ptr_m->prev;
    ptr_remain->next = ptr_m->next;
    if (ptr_m->prev != NULL)
      ptr_m->prev->next = ptr_remain;
    if (ptr_m->next != NULL)
      ptr_m->next->prev = ptr_remain;
    if (freeList == ptr_m) //only one element in list
      freeList = ptr_remain;
  }
  else
  { //no remain  delete this node from list
    if (ptr_m->prev != NULL && ptr_m->next != NULL)
    { //delete middle element
      ptr_m->prev->next = ptr_m->next;
      ptr_m->next->prev = ptr_m->prev;
    }
    else if (ptr_m->prev == NULL && ptr_m->next != NULL)
    { //delete the first element
      ptr_m->next->prev = NULL;
      freeList = ptr_m->next;
    }
    else if (ptr_m->prev != NULL && ptr_m->next == NULL)
    { //delete the last element
      ptr_m->prev->next = NULL;
    }
    else
    { //only one element in list
      freeList == NULL;
    }
  }

  //set metadata for unfree node
  ptr_m->size = size;
  ptr_m->next = NULL;
  ptr_m->prev = NULL;

  return ptr_m;
}

////////////////////////////Test Funtions//////////////////////////
unsigned long get_data_segment_size()
{
  return heapSize;
}

unsigned long get_data_segment_free_space_size()
{
  unsigned long freeSpace = 0;
  for (metaInfo *ptr = freeList; ptr != NULL; ptr = ptr->next)
  {
    freeSpace += ptr->size + sizeof(metaInfo);
  }
  return freeSpace;
}

////////////////////////////Debug Funtions//////////////////////////
void printLinkedList()
{
  int len = 0;
  for (metaInfo *ptr = freeList; ptr != NULL; ptr = ptr->next)
  {
    printf("node: size(%zu),  prev(%p), cur_address(%lu), next(%lu)\n", ptr->size, ptr->prev, ptr, ptr->next);
    len++;
  }
  printf("the length of list is (%d)\n", len);
  printf("---------------------------------------------------\n");
  printf("\n");
}

void getListLength()
{
  int len = 0;
  for (metaInfo *ptr = freeList; ptr != NULL; ptr = ptr->next)
    len++;
  printf("the length of list is (%d)\n", len);
  printf("---------------------------------------------------\n");
  printf("\n");
}

int cycleDetect()
{
  metaInfo *fast = freeList;
  metaInfo *slow = freeList;

  while (fast != NULL && fast->next != NULL)
  {
    fast = fast->next->next;
    slow = slow->next;
    if (fast == slow)
      return 1; //cycle exist
  }
  return 0;
}