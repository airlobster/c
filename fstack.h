
#ifndef _SSTACK_H_
#define _SSTACK_H_

#include <stdlib.h>

#define	FSTACK_EMPTY			((void*)-1L)

typedef void fstack_t;
typedef void(*stack_dtor)(void* data);

void stack_init(fstack_t** s, stack_dtor dtor);
void stack_deinit(fstack_t* s);
void stack_push(fstack_t* s, void* data);
void* stack_pop(fstack_t* s);
void* stack_peek(fstack_t* s);
size_t stack_size(fstack_t* s);
void stack_enum(fstack_t* s, void(*f)(void* e, void* user), void* user);
void stack_enum_rev(fstack_t* s, void(*f)(void* e, void* user), void* user);
int stack_empty(fstack_t* s);

#endif
