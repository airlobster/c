#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdlib.h>

typedef void buffer_t;

void buffer_init(buffer_t** b);
void buffer_destroy(buffer_t* b);
void buffer_reset(buffer_t* b);
const char* buffer_get(buffer_t* b);
void buffer_append(buffer_t* b, char c);
void buffer_append_s(buffer_t* b, const char* s);
void buffer_append_n(buffer_t* b, const char* s, int n);
void buffer_append_rep(buffer_t* b, char c, int repeat);
void buffer_append_va(buffer_t* b, const char* fmt, ...);
void buffer_setlength(buffer_t* b, int l);
int buffer_pop(buffer_t* b);
size_t buffer_length(buffer_t* b);
char* buffer_detach(buffer_t* b);

#endif
