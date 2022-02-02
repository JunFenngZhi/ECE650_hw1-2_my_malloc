#ifndef  MYALLOC_H
#define  MYALLOC_H

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>



//Thread Safe malloc/free: locking version 
void *ts_malloc_lock(size_t size); 
void ts_free_lock(void *ptr);

//Thread Safe malloc/free: non-locking version 
void *ts_malloc_nolock(size_t size); 
void ts_free_nolock(void *ptr);

typedef struct metaInfo_t {
  size_t size;
  struct metaInfo_t * prev;
  struct metaInfo_t * next;
}metaInfo;



#endif