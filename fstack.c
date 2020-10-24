
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "fstack.h"

#define	SSTACK_PAGE_SIZE	(1024)

typedef struct _sstack_impl_t {
	void** a;
	size_t size;
	size_t wr;
	stack_dtor dtor;
} sstack_impl_t;


void stack_init(fstack_t** s, stack_dtor dtor) {
	sstack_impl_t* si = ALLOC(sstack_impl_t);
	si->a = 0;
	si->size = 0;
	si->wr = 0;
	si->dtor = dtor;
	*s = si;
}

void stack_deinit(fstack_t* s) {
	sstack_impl_t* si = (sstack_impl_t*)s;
	while( si->wr ) {
		void* data = stack_pop(s);
		if( data && si->dtor )
			si->dtor(data);
	}
	if( si->a )
		FREE(si->a);
	FREE(si);
}

void stack_push(fstack_t* s, void* data) {
	sstack_impl_t* si = (sstack_impl_t*)s;
	if( si->wr == si->size ) {
		si->size += SSTACK_PAGE_SIZE;
		si->a = REALLOC(si->a, void*, si->size);
	}
	si->a[si->wr++] = data;
}

void* stack_pop(fstack_t* s) {
	sstack_impl_t* si = (sstack_impl_t*)s;
	void* data = FSTACK_EMPTY;
	if( si->wr )
		data = si->a[--si->wr];
	return data;
}

void* stack_peek(fstack_t* s) {
	sstack_impl_t* si = (sstack_impl_t*)s;
	return si->wr ? si->a[si->wr - 1] : FSTACK_EMPTY;
}

size_t stack_size(fstack_t* s) {
	sstack_impl_t* si = (sstack_impl_t*)s;
	return si->wr;
}

void stack_enum(fstack_t* s, void(*f)(void* e, void* user), void* user) {
	sstack_impl_t* si = (sstack_impl_t*)s;
	int i;
	for(i=0; i < si->wr; i++) {
		f(si->a[i], user);
	}
}

int stack_empty(fstack_t* s) {
	sstack_impl_t* si = (sstack_impl_t*)s;
	return si->wr == 0;
}

