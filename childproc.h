
#ifndef _CHILDPROC_H_
#define _CHILDPROC_H_

#include <stdio.h>
#include <unistd.h>

typedef enum _stream_type_t {
	stSTDOUT=STDOUT_FILENO,
	stSTDERR=STDERR_FILENO
} stream_type_t;

typedef int(*child_process_callback_t)(stream_type_t type, const char* pch, size_t n, void* user);

int child_process_exec(const char* cmd, FILE* fin, child_process_callback_t cb, void* user);

#endif
