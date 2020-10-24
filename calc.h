// calc.h

#ifndef _CALC_H_
#define _CALC_H_

typedef enum _calc_status_t {
	CS_OK=0,
	CS_BAD_EXPRESSION,
	CS_EMPTY_EXPRESSION,
	CS_NAN,
	CS_INFINITE,
	CS_BAD_NUMBER,
	CS_UNKNOWN_OPERATOR,
	CS_BAD_OPERAND,
	CS_UNKNOWN_FUNCTION,
	CS_UNKNOWN_VARIABLE,
	CS_NOT_INTEGER,
	CS_NOT_UNSIGNED,
	CS_EXTRA_OPEN_PAREN,
	CS_EXTRA_CLOSE_PAREN,
	CS_LOW_ARGS,
	CS_HIGH_ARGS,
	CS_NO_SYMBOLS,
	CS_CONSTANT_VAR,
} calc_status_t;

typedef long double number_t;

typedef struct _function_info_t {
	const char* name;
	const char* description;
	int minArgs, maxArgs;
	int(*f)(const char* name, size_t nargs, const number_t* args, number_t* result);
} function_info_t;

typedef struct _calc_symboltable_t {
	int (*getvar)(struct _calc_symboltable_t* st, const char* name, number_t* value);
	int (*setvar)(struct _calc_symboltable_t* st, const char* name, number_t value);
	int (*enumvars)(struct _calc_symboltable_t* st, int(*cb)(const char* name, number_t value, void* context), void* context);
	void (*resetvars)(struct _calc_symboltable_t* st);
	int (*execfunc)(struct _calc_symboltable_t* st, const char* name, size_t nargs, const number_t* args, number_t* result);
	int (*enumfuncs)(struct _calc_symboltable_t* st, int(*cb)(const function_info_t* fi, void* context), void* context);
} calc_symboltable_t;

int calc_evaluate(const char* expression, calc_symboltable_t* symb, void* cookie, number_t* result);
const char* calc_get_error(int err);

#endif
