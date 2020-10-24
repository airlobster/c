
#include <unistd.h>
#include "utils.h"

static void _print_safe(FILE* f, const char* s) {
	int redirected = ! isatty(fileno(f));
	int state=0;
	while( *s ) {
		if( state==0 ) {
			if( *s=='\e' && redirected ) {
				--s;
				state=1;
			} else
			{
				fputc(*s, f);
			}
		} else {
			if( *s == 'm' )
				state=0;
		}
		++s;
	}
}

int cfprintf(FILE* f, const char* fmt, ...) {
	int i;
	va_list start;
	char* s;

	va_start(start, fmt);

	i=vasprintf(&s, fmt, start);
	_print_safe(f, s);
	free(s);

	va_end(start);

	return i;
}

int cafprintf(FILE* f, const char* fmt, va_list va) {
	int i;
	char* s;

	i=vasprintf(&s, fmt, va);
	_print_safe(f, s);
	free(s);

	return i;
}
