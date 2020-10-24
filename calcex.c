// calcex.c

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "buffer.h"
#include "hashtable.h"
#include "a_debug.h"
#include "childproc.h"
#include "calcex.h"

typedef struct _extensions_t {
	calc_symboltable_t itf;
	hashtable_t* htVars;
} extensions_t;

// CONSTANTS
typedef struct _constant_t {
	const char* name;
	number_t value;
} constant_t;
static const constant_t _constants[] = {
	{ "PI", M_PI },
	{ "E", M_E },
	{ 0, 0 }
};

// FUNCTIONS
static int _fh_rnd(const char* name, size_t nargs, const number_t* args, number_t* result) {
	uint64_t v;
	arc4random_buf(&v, sizeof(v));
	*result = v;
	return CS_OK;
}

static const function_info_t _functions[] = {
	{ "rnd", 0, 0, 0, _fh_rnd },
	{ 0, 0, 0, 0, 0 }
};


static number_t* _make_number(number_t v) {
	number_t* pv = (number_t*)malloc(sizeof(number_t));
	*pv = v;
	return pv;
}

static int _getvar(struct _calc_symboltable_t* st, const char* name, number_t* value) {
	extensions_t* x = (extensions_t*)st;

	const constant_t* c;
	for(c=_constants; c->name; c++) {
		if( strcasecmp(name, c->name)==0 ) {
			*value = c->value;
			return CS_OK;
		}
	}

	number_t* pv = (number_t*)hashtable_get(x->htVars, name);
	if( ! pv )
		return CS_UNKNOWN_VARIABLE;
	*value = *pv;
	return CS_OK;
}

static int _setvar(struct _calc_symboltable_t* st, const char* name, number_t value) {
	extensions_t* x = (extensions_t*)st;

	const constant_t* c;
	for(c=_constants; c->name; c++) {
		if( strcasecmp(name, c->name)==0 )
			return CS_CONSTANT_VAR;
	}

	hashtable_put(x->htVars, name, _make_number(value));
	return CS_OK;
}

static int _cb_external(stream_type_t type, const char* pch, size_t n, void* user) {
	buffer_t* out = (buffer_t*)user;
	if( type == stSTDOUT ) {
		buffer_append_n(out, pch, n);
	} else {
		fwrite(pch, 1, n, stderr);
		fflush(stderr);
	}
	return 0;
}
static int _execExternalFunc(const char* name, size_t nargs, const number_t* args, number_t* result) {
	int err=0, i;
	buffer_t *cmd, *out;
	const char* path = getenv("CALC_PATH") ? getenv("CALC_PATH") : ".";
	ASSERT(result);
	*result = 0;
	buffer_init(&cmd);
	buffer_init(&out);
	// build command
	buffer_append_va(cmd, "%s/%s", path, name);
	for(i=0; i < nargs; i++)
		buffer_append_va(cmd, " %Lg", args[i]);
	// execute
	if( (err=child_process_exec(buffer_get(cmd), 0, _cb_external, out)) == 0 ) {
		char* end;
		*result = strtold(buffer_get(out), &end);
		while( isspace(*end) )
			++end;
		err = *end ? CS_BAD_NUMBER : CS_OK;
	} else {
		err = CS_UNKNOWN_FUNCTION;
	}
	buffer_destroy(cmd);
	buffer_destroy(out);
	return err;
}
static int _execfunc(struct _calc_symboltable_t* st, const char* name, size_t nargs, const number_t* args, number_t* result) {
	// extensions_t* x = (extensions_t*)st;
	const function_info_t* p;
	*result = 0;
	for(p=_functions; p->name; p++) {
		ASSERT(p->f);
		if( strcasecmp(p->name, name)==0 )
			return (*p->f)(name, nargs, args, result);
	}
	// if function not found as a builtin function, try running it as an external one
	return _execExternalFunc(name, nargs, args, result);
}

static void _resetvars(struct _calc_symboltable_t* st) {
	extensions_t* x = (extensions_t*)st;
	hashtable_reset(x->htVars);
}

typedef struct _enumvars_context_proxy_t {
	int(*cb)(const char* name, number_t value, void* context);
	void* context;
} enumvars_context_proxy_t;
static int _enumvars_cb_proxy(const char* key, void* value, void* cookie) {
	enumvars_context_proxy_t* pxy = (enumvars_context_proxy_t*)cookie;
	return (*pxy->cb)(key, *(number_t*)value, pxy->context);
}
static int _enumvars(struct _calc_symboltable_t* st, int(*cb)(const char* name, number_t value, void* context), void* context) {
	enumvars_context_proxy_t pxy={ .cb=cb, .context=context };
	extensions_t* x = (extensions_t*)st;
	hashtable_enum_sorted(x->htVars, 0, _enumvars_cb_proxy, &pxy);
	return CS_OK;
}


int calcext_init_extensions(calc_symboltable_t** ext) {
	extensions_t* x = (extensions_t*)malloc(sizeof(extensions_t));
	x->itf.execfunc = _execfunc;
	x->itf.getvar = _getvar;
	x->itf.setvar = _setvar;
	x->itf.enumvars = _enumvars;
	x->itf.resetvars = _resetvars;
	x->itf.enumfuncs = 0;
	hashtable_init(free, &x->htVars);
	*ext = (calc_symboltable_t*)x;
	return CS_OK;
}

void calcext_free_extensions(calc_symboltable_t* ext) {
	extensions_t* x = (extensions_t*)ext;
	hashtable_deinit(x->htVars);
	free(ext);
}
