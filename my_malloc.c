#include "my_malloc.h"


void insList(metaInfo *ptr_m, const int HasLock);
void *makeNewBlock(const size_t size, const int HasLock);
void *unFreeOldBlock(metaInfo *ptr_m, const size_t size, const int HasLock);
void my_free(void *ptr, const int HasLock);
metaInfo *mergeBlock(metaInfo *p1_m, metaInfo *p2_m);


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
size_t heapSize = 0;
metaInfo *freeList_lock = NULL; //head of freeList
__thread metaInfo *freeList_nolock = NULL;


////////////////////Thread Safe malloc and free (lock version)/////////////////////////////
void *ts_malloc_lock(size_t size){
  if(size == 0)
    return NULL;

  pthread_mutex_lock(&lock);
  void *ptr = NULL;

  //search freeList to allocation proper block(Best fit policy)
  metaInfo *ptr_BF = NULL;
  size_t minDiff = __SIZE_MAX__;
  for (metaInfo *ptr_m = freeList_lock; ptr_m != NULL; ptr_m = ptr_m->next)
  {
    if (ptr_m->size >= size && ptr_m->size - size < minDiff)
    {
      minDiff = ptr_m->size - size;
      ptr_BF = ptr_m;
    }
    if (minDiff == 0)
      break;
  }

  //allocate new block
  if (ptr_BF != NULL)     //find fitting block
    ptr = unFreeOldBlock(ptr_BF, size, 1);
  else    //no fitting block
    ptr = makeNewBlock(size, 1);
  
  pthread_mutex_unlock(&lock);
  return ptr + sizeof(metaInfo);
} 

void ts_free_lock(void *ptr){
  pthread_mutex_lock(&lock);
  my_free(ptr,1);
  pthread_mutex_unlock(&lock);
}


////////////////////Thread Safe malloc and free (unlock version)///////////////////////////
void *ts_malloc_nolock(size_t size){
  if(size == 0)
    return NULL;

  void *ptr = NULL;

  //search freeList to allocation proper block(Best fit policy)
  metaInfo *ptr_BF = NULL;
  size_t minDiff = __SIZE_MAX__;
  for (metaInfo *ptr_m = freeList_nolock; ptr_m != NULL; ptr_m = ptr_m->next)
  {
    if (ptr_m->size >= size && ptr_m->size - size < minDiff)
    {
      minDiff = ptr_m->size - size;
      ptr_BF = ptr_m;
    }
    if (minDiff == 0)
      break;
  }

  //allocate new block
  if (ptr_BF != NULL)  //find fitting block
    ptr = unFreeOldBlock(ptr_BF, size, 0);
  else    //no fitting block
    ptr = makeNewBlock(size, 0);

  return ptr + sizeof(metaInfo);
} 

void ts_free_nolock(void *ptr){
  my_free(ptr,0);
}


////////////////////////////Helper Functions//////////////////////////
void my_free(void *ptr, const int HasLock)
{
  assert(ptr!=NULL);

  metaInfo *ptr_m = ptr - sizeof(metaInfo);
  insList(ptr_m, HasLock);

  //merge adjacent free block
  metaInfo *prev = ptr_m->prev;
  metaInfo *next = ptr_m->next;
  metaInfo *merge_ptr = mergeBlock(prev, ptr_m);
  mergeBlock(merge_ptr, next);

  return;
}

metaInfo *mergeBlock(metaInfo *p1_m, metaInfo *p2_m) //p1_m points to the front block, p2_m points to the next block
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

void insList(metaInfo *ptr_m, const int HasLock)
{
  metaInfo *prev = NULL;
  metaInfo **cur = NULL;
  if(HasLock == 1)
    cur = &freeList_lock;
  else
    cur = &freeList_nolock;  
  
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

void *makeNewBlock(const size_t size, const int HasLock)
{
  size_t blockSize = size + sizeof(metaInfo); //new block size
  void * ptr = NULL;
  if(HasLock == 1){
    ptr = sbrk((intptr_t)blockSize);
  }
  else{
    //printf("prepare to call sbrk()\n");
    pthread_mutex_lock(&lock);
    ptr = sbrk((intptr_t)blockSize);
    pthread_mutex_unlock(&lock);
    //printf("already call sbrk()\n");
  }
  assert(ptr != NULL);

  metaInfo *ptr_m = (metaInfo *)ptr; // set new block's meta data
  ptr_m->size = size;
  ptr_m->next = NULL;
  ptr_m->prev = NULL;

  heapSize += blockSize;

  return ptr;
}

void *unFreeOldBlock(metaInfo *ptr_m, const size_t size, const int HasLock)
{
  metaInfo** freeList = NULL;
  if(HasLock == 1)
    freeList = &freeList_lock;
  else
    freeList = &freeList_nolock;

  //printf("ptr_m->size:%lu, size:%lu, sizeof(metaInfo):%lu \n", ptr_m->size,size,sizeof(metaInfo));
  if (ptr_m->size > size + sizeof(metaInfo))
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
    if (*freeList == ptr_m) 
      *freeList = ptr_remain;
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
      *freeList = ptr_m->next;
    }
    else if (ptr_m->prev != NULL && ptr_m->next == NULL)
    { //delete the last element
      ptr_m->prev->next = NULL;
    }
    else
    { //only one element in list
      *freeList = NULL;
    }
  }

  //set metadata for unfree node
  ptr_m->size = size;
  ptr_m->next = NULL;
  ptr_m->prev = NULL;

  return ptr_m;
}