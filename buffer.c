
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "utils.h"
#include "buffer.h"

#define	BUFFER_PAGE_SIZE	(1024)

typedef struct _buffer_impl_t {
	char* b;
	int size;
	int wr;
} buffer_impl_t;


void buffer_init(buffer_t** b) {
	buffer_impl_t* bi = ALLOC(buffer_impl_t);
	bi->b = 0;
	bi->size = 0;
	bi->wr = 0;
	*b = bi;
}

void buffer_destroy(buffer_t* b) {
	buffer_impl_t* bi = (buffer_impl_t*)b;
	if( bi->b )
		free(bi->b);
	FREE(bi);
}

void buffer_reset(buffer_t* b) {
	buffer_impl_t* bi = (buffer_impl_t*)b;
	bi->wr = 0;
	if( bi->b )
		bi->b[bi->wr] = 0;
}

const char* buffer_get(buffer_t* b) {
	buffer_impl_t* bi = (buffer_impl_t*)b;
	if( ! bi->wr )
		return "";
	return bi->b;
}

void buffer_append(buffer_t* b, char c) {
	buffer_impl_t* bi = (buffer_impl_t*)b;
	if( bi->wr >= bi->size - 2 ) {
		bi->size += BUFFER_PAGE_SIZE;
		bi->b = REALLOC(bi->b,char,bi->size);
	}
	bi->b[bi->wr++] = c;
	bi->b[bi->wr] = 0;
}

void buffer_append_s(buffer_t* b, const char* s) {
	if( ! s )
		return;
	while( *s )
		buffer_append(b, *s++);
}

void buffer_append_n(buffer_t* b, const char* s, int n) {
	if( ! s )
		return;
	while( *s && n-- > 0 )
		buffer_append(b, *s++);
}

void buffer_append_rep(buffer_t* b, char c, int repeat) {
	while( repeat-- > 0 )
		buffer_append(b, c);
}

void buffer_setlength(buffer_t* b, int l) {
	buffer_impl_t* bi = (buffer_impl_t*)b;
	if( l >= bi->size )
		return;
	bi->wr = l;
	bi->b[bi->wr] = 0;
}

size_t buffer_length(buffer_t* b) {
	buffer_impl_t* bi = (buffer_impl_t*)b;
	return bi->wr;
}

char* buffer_detach(buffer_t* b) {
	buffer_impl_t* bi = (buffer_impl_t*)b;
	char* t = bi->b;
	bi->b = 0;
	bi->size = 0;
	bi->wr = 0;
	return t;
}

int buffer_pop(buffer_t* b) {
	buffer_impl_t* bi = (buffer_impl_t*)b;
	if( bi->wr == 0 )
		return -1;
	int c = bi->b[--bi->wr];
	bi->b[bi->wr] = 0;
	return c;
}

void buffer_append_va(buffer_t* b, const char* fmt, ...) {
	va_list va;
	char* s;
	va_start(va, fmt);
	vasprintf(&s, fmt, va);
	buffer_append_s(b, s);
	free(s);
	va_end(va);
}

