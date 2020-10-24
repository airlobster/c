// dlist.h

#ifndef _DLIST_H_
#define _DLIST_H_

#include <stdlib.h>

typedef void dlist_t;
typedef void(*dlist_dtor)(void*);

void dlist_init(dlist_dtor dtor, dlist_t** l);
void dlist_deinit(dlist_t* l);
void dlist_reset(dlist_t* l);
size_t dlist_len(dlist_t* l);
void dlist_push_back(dlist_t* l, void* e);
void dlist_push_front(dlist_t* l, void* e);
int dlist_enum(dlist_t* l, int(*cb)(void* data, void* cookie), void* cookie);
void dlist_detach(dlist_t* l, size_t* n, void*** a);

#endif
