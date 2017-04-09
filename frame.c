#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kvi.h"

/***********************************************
 *           memory opeartion
 ************************************************/
void *realloc_cache(cache *c){

  int size = c->size << 1;
  char *new_buffer;

  saft_malloc(new_buffer, INIT_SIZE * sizeof(char), c, NULL);

  memset(new_buffer, '\0', size);
  memcpy(c->begin, new_buffer, c->size);

  c->end = new_buffer + (c->end - c->begin);
  c->cur = new_buffer + (c->cur - c->begin);
  c->size = size;

  saft_free(c->begin);
  c->begin = new_buffer;

  return c->begin;
}


cache *create_cache() {

  cache *c;
  saft_malloc(c, sizeof(cache), c, NULL);
  c = (cache *)c;

  char *buffer;
  saft_malloc(buffer, INIT_SIZE * sizeof(char), c, NULL);
  buffer = (char *)buffer;

  memset(buffer, '\0', INIT_SIZE);
  c->size = INIT_SIZE;
  c->begin = c->end = c->cur = buffer;
  c->colidx = c->rowidx = 0;

  return c;
}


frame *add_frame(global *kVI, char *filename) {

  frame *n;

  saft_malloc(n, sizeof(frame), n, NULL);
  n = (frame *)n;

  memset(n, 0, sizeof(frame));

  n->filename = filename;
  n->extension = "";

  n->next = kVI->head.next;
  kVI->head.next = n;

  n->cache = create_cache();

  if (n->cache)
    return n;

  // malloc fialed
  kVI->head.next = n->next;
  saft_free(n);

  return NULL;
}


/* free all frames  */
void free_frames(global *kVI) {

  for (frame *p = kVI->head.next; p;) {
    kVI->head.next = p->next;

    saft_free(p->history);
    saft_free(p->syntax_highlight);
    saft_free_cache(p->cache);

    saft_free(p);
    p = kVI->head.next;
  }

  kVI->head.next = NULL;
}


/***********************************************
 *           copy bidirction
 ************************************************/
void copy_forward(char *src, char *dst, int size) {

  for (int i = 0; i < size; i++) {
    *(dst + i) = *(src + i);
  }
  *(dst + size - 1) = '\0';
}


void copy_backward(char *src, char *dst, int size) {

  for (int i = size - 1 ; i >= 0; i--) {
    *(dst + i) = *(src + i);
  }
  *(dst + size + 1) = '\0';
}
