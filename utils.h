// utils.h

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define	ALLOC(type)				((type*)malloc(sizeof(type)))
#define	ALLOCA(type,n)			((type*)malloc(sizeof(type)*(n)))
#define REALLOC(p,type,n)		((type*)realloc((p),sizeof(type)*(n)))
#define FREE(p)					free(p)


// print text with ANSI escape codes.
// the color codes will be ignored if the output file is not TTY
int cfprintf(FILE* f, const char* fmt, ...);
int cafprintf(FILE* f, const char* fmt, va_list va);

#endif
