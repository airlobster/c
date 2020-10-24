
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include "utils.h"
#include "a_debug.h"

static __thread int _nest = 0;

void _trace(const char* func, const char* file, int line, const char* fmt, ...) {
	static const char* const fmtColored = "[%p][\e[92m%s\e[0m@%s:%d]: \e[93m%s\e[0m\n";
	char *_fmt;
	int i;
	va_list start;

	va_start(start, fmt);

	// enrich the format string with more info
	asprintf(&_fmt, fmtColored,	pthread_self(), func, file, line, fmt);

	// indent
	for(i=0; i < _nest; i++)
		fprintf(stderr, "%s", TRACE_INDENT);

	cafprintf(stderr, _fmt, start);

	free(_fmt);

	va_end(start);
}

void _begin_function(const char* func, const char* file, int line) {
	_trace(func, file, line, "BEGIN");
	_nest++;
}
void _end_function(const char* func, const char* file, int line) {
	if( _nest > 0 )
		--_nest;
	_trace(func, file, line, "END");
}
