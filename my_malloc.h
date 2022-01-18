#ifndef  MYALLOC_H
#define  MYALLOC_H

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

//first fit malloc() and free()
void * ff_malloc(size_t size);
void ff_free(void * ptr);

//best fit malloc() and free()
void * bf_malloc(size_t size);
void bf_free(void * ptr);

typedef struct metaInfo_t {
  size_t size;
  struct metaInfo_t * prev;
  struct metaInfo_t * next;
}metaInfo;

//test function
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes

//Debug Funtion
void printLinkedList();
int cycleDetect();
void getListLength();


#endif