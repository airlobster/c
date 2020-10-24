
#ifndef _DEBUG_H_
#define _DEBUG_H_

#define	TRACE_INDENT	("\t")

#ifdef _DEBUG_
#include <assert.h>
extern void _trace(const char* func, const char* file, int line, const char* fmt, ...);
extern void _begin_function(const char* func, const char* file, int line);
extern void _end_function(const char* func, const char* file, int line);
#define TRACE(fmt, ...)			_trace(__FUNCTION__, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define TRACEE()				TRACE("")
#define ASSERT(expr)			assert(expr)
#define BEGIN_FUNCTION()		_begin_function(__FUNCTION__, __FILE__, __LINE__)
#define END_FUNCTION()			_end_function(__FUNCTION__, __FILE__, __LINE__)
#define NOT_IMPLEMENTED(item)	TRACE("%s not implemented yet!", (item))
#else
#define TRACE(fmt, ...)
#define TRACEE()
#define ASSERT(expr)
#define BEGIN_FUNCTION()
#define END_FUNCTION()
#define NOT_IMPLEMENTED(item)
#endif

#endif
