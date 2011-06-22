/* Wrapper functions for malloc/free that force 16-byte alignment
 * See http://perso.club-internet.fr/matmac/sourcesc.htm

 * Copyright 2001 Phil Karn, KA9Q
 * May be used under the terms of the GNU Public License (GPL)
 */

#include <malloc16.h>
#include <string.h>
typedef unsigned long UintPtr;

void* malloc16Align(int BlockSize)
	{
	/* allocates the memory needed for a block of memory
	 *, plus one header bytes that indicates the true start of the malloc'ed block */
	
	UintPtr BasePtr=(UintPtr)malloc(BlockSize+32);
	UintPtr AlignedPtr=(BasePtr+31)&-16;
	
	/* we need at least 1 byte of valid memory before the aligned block
	 * that byte will store the "padding" which is before the aligned block
	 * so we can free correctly the block of memory */
	
	if (AlignedPtr==BasePtr) AlignedPtr+=16;
	
	/* stores that famous byte */
	*((char*)AlignedPtr-1)=(char)(AlignedPtr-BasePtr);
	
	return (void *)AlignedPtr;
	}

#if 0
void *malloc16Align(int size){
  void *p;
  void **p1;

  if((p = malloc(size+31)) == NULL)
    return NULL;

  /* Round up to next 16-byte boundary */
  p1 = (void **)(((long)p + 31) & (~15));

  /* Stash actual start of block just before ptr we return */
  p1[-1] = p;

  /* Return 16-byte aligned address */
  return (void *)p1;
}
#endif
void *calloc16Align(size_t nmemb,size_t size){
  int nbytes;
  void *p;

  nbytes = nmemb*size;
  if((p = malloc16Align(nbytes)) == NULL)
    return NULL;

  memset(p,0,nbytes);
  return p;
}
void free16Align(void* p)
	{
	char* pAligned=(char*)p;
	
	/* *(pAligned-1) is the number of bytes just before the aligned block
	 * so, pAligned - *(pAligned-1) is the original block of memory
	 */
	free(pAligned - *(pAligned-1));
	}
#if 0
void free16Align(void *p){

  if(p != NULL){
    /* Retrieve pointer to actual start of block and free it */
    free(((void **)p)[-1]);
  }
}
#endif