// calc.c

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "a_debug.h"
#include "fstack.h"
#include "buffer.h"
#include "dlist.h"
#include "lexer.h"
#include "calc.h"

#define MAX_FUNC_ARGS (64)

typedef enum _operator_flags_t {
	ofInteger=0x1,
	ofUnsigned=0x2,
	ofBitwise=(ofInteger|ofUnsigned),
} operator_flags_t;

typedef struct _operator_info_t {
	int type;
	const char* symbol;
	int precendence;
	unsigned int flags;
	size_t minArgs, maxArgs;
	int (*f)(void* context, size_t n, const number_t* args, number_t* ret);
} operator_info_t;


typedef struct _calc_context_t {
	lexer_t* lexer;
	lexer_token_t** aPostfix;
	size_t nPostfix;
	fstack_t* postfixResult;
	buffer_t* postfixString;
	calc_symboltable_t* symb;
	void* user;
	const operator_info_t* oplookup[0x1000];
} calc_context_t;


static int oh_add(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=args[0] + args[1]; return 0; }
static int oh_sub(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=args[0] - args[1]; return 0; }
static int oh_mul(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=args[0] * args[1]; return 0; }
static int oh_div(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=args[0] / args[1]; return 0; }
static int oh_rem(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=(uint64_t)args[0] % (uint64_t)args[1]; return 0; }
static int oh_bwnot(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=~(uint64_t)args[0]; return 0; }
static int oh_xor(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=(uint64_t)args[0] ^ (uint64_t)args[1]; return 0; }
static int oh_and(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=(uint64_t)args[0] & (uint64_t)args[1]; return 0; }
static int oh_or(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=(uint64_t)args[0] | (uint64_t)args[1]; return 0; }
static int oh_shl(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=(uint64_t)args[0] << (uint64_t)args[1]; return 0; }
static int oh_shr(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=(uint64_t)args[0] >> (uint64_t)args[1]; return 0; }
static int oh_pow(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=powl(args[0], args[1]); return 0; }
static int oh_root(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=powl(args[0], 1/args[1]); return 0; }
static int oh_neg(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=-args[0]; return 0; }
static int oh_pos(void* context, size_t n, const number_t* args, number_t* ret)
	{ *ret=args[0]; return 0; }


static inline const operator_info_t* _resolve_operator(calc_context_t* c, int type) {
	TRACE("Resolving operator 0x%X", type);
	const operator_info_t* oi = c->oplookup[type];
	if( ! oi ) {
		TRACE("Failed to resolve operator 0x%X", type);
	}
	return oi;
}

static const operator_info_t _opinfo[] = {
	{ ttAssign,		"=",	0,		0,			1, 1,	0 },
	{ ',',			"",		100, 	0,			1, 1,	0 },
	{ '|',			"|",	200, 	ofBitwise,	2, 2,	oh_or },
	{ '^',			"^",	300, 	ofBitwise,	2, 2,	oh_xor },
	{ '&',			"&",	400, 	ofBitwise,	2, 2,	oh_and },
	{ ttSHL,		"<<",	500, 	ofBitwise,	2, 2,	oh_shl },
	{ ttSHR,		">>",	500, 	ofBitwise,	2, 2,	oh_shr },
	{ '+',			"+",	600, 	0,			2, 2,	oh_add },
	{ '-',			"-",	600, 	0,			2, 2,	oh_sub },
	{ '*',			"*",	700, 	0,			2, 2,	oh_mul },
	{ '/',			"/",	700, 	0,			2, 2,	oh_div },
	{ '%',			"%",	700, 	ofInteger,	2, 2,	oh_rem },
	{ ttPower,		"**",	800, 	0,			2, 2,	oh_pow },
	{ ttRoot,		"//",	800, 	0,			2, 2,	oh_root },
	{ ttUMinus,		"NEG",	900, 	0,			1, 1,	oh_neg },
	{ ttUPlus,		"POS",	900, 	0,			1, 1,	oh_pos },
	{ '~',			"~",	900, 	ofBitwise,	1, 1,	oh_bwnot },
	{ ttFunction,	0,		1000, 	0,			0, 0,	0 },
};

// operators predicate function for infix2postfix
static int _compare_op(const lexer_token_t* t1, const lexer_token_t* t2, void* context, int* ret) {
	calc_context_t* c = (calc_context_t*)context;
	ASSERT(ret);
	const operator_info_t* oi1 = _resolve_operator(c, t1->type);
	if( ! oi1 )
		return CS_UNKNOWN_OPERATOR;
	const operator_info_t* oi2 = _resolve_operator(c, t2->type);
	if( ! oi2 )
		return CS_UNKNOWN_OPERATOR;
	*ret = oi1->precendence - oi2->precendence;
	return CS_OK;
}

static int _strtonumber(const char* s, int base, number_t* ret) {
	char* end;
	int err=CS_OK;
	*ret = 0;
	ASSERT(base==10 || base==2 || base==8 || base==16);
	if( base == 10 ) {
		*ret = strtold(s, &end);
		if( isnan(*ret) )
			err = CS_NAN;
		else if( isinf(*ret) )
			err = CS_INFINITE;
		else if( *end )
			err = CS_BAD_NUMBER;
	} else {
		*ret = strtoull(s, &end, base);
		if( *end )
			return CS_BAD_NUMBER;
	}
	return err;
}

static int _translate_operand(calc_context_t* c, const lexer_token_t* t, number_t* ret) {
	int err = CS_OK;
	ASSERT(IS_LEXER_OPERAND(t->type));
	switch( t->type ) {
		case ttNumber: {
			err=_strtonumber(t->text, 10, ret);
			break;
		}
		case ttBinary: {
			err=_strtonumber(t->text, 2, ret);
			break;
		}
		case ttOctal: {
			err=_strtonumber(t->text, 8, ret);
			break;
		}
		case ttHex: {
			err=_strtonumber(t->text, 16, ret);
			break;
		}
		case ttLastFuncArg: {
			*ret = nanl("");
			break;
		}
		case ttVariable: {
			*ret=0;
			if( c->symb && c->symb->getvar ) {
				err = (*c->symb->getvar)(c->symb, t->text, ret);
			} else {
				err = CS_NO_SYMBOLS;
			}
			break;
		}
		default: {
			ASSERT(0);
			break;
		}
	}
	return err;
}

static number_t* _make_number(number_t v) {
	number_t* pld = (number_t*)malloc(sizeof(number_t));
	*pld = v;
	return pld;
}

static int _cb_evaluate(void* data, void* cookie) {
#define isinteger(v)	(fpclassify(fmod((v), 1))==FP_ZERO)
	BEGIN_FUNCTION();
	calc_context_t* x = (calc_context_t*)cookie;
	lexer_token_t* t = (lexer_token_t*)data;
	number_t ld;
	int err = 0, i;

	TRACE("token type=0x%03X (%d) '%s'", t->type, t->type, t->text ? t->text : "");

	if( IS_LEXER_OPERAND(t->type) ) {
		// OPERANDS
		if( (err=_translate_operand(x, t, &ld)) == CS_OK )
			stack_push(x->postfixResult, _make_number(ld));
	} else {
		// OPERATORS
		switch( t->type ) {
			case '(': {
				// this open-paren should have been removed by now by a closing-paren,
				// during infix2postfix
				err = CS_EXTRA_OPEN_PAREN;
				break;
			}
			case ttAssign: {
				if( stack_size(x->postfixResult) < 1 ) {
					err = CS_BAD_EXPRESSION;
					break;
				}
				if( x->symb && x->symb->setvar ) {
					number_t* a1 = (number_t*)stack_peek(x->postfixResult);
					err = (*x->symb->setvar)(x->symb, t->text, *a1);
				} else {
					err = CS_NO_SYMBOLS;
				}
				break;
			}
			case ttFunction: {
				int nargs=0;
				number_t args[MAX_FUNC_ARGS];
				while( ! stack_empty(x->postfixResult) ) {
					number_t* pv = (number_t*)stack_pop(x->postfixResult);
					if( isnan(*pv) ) { // func args terminator?
						free(pv);
						break;
					}
					args[nargs++] = *pv;
					free(pv);
				}
				// create a reversed version of the args array
				number_t argsr[nargs];
				for(i=0; i < nargs; i++)
					argsr[nargs-i-1] = args[i];

				if( x->symb && x->symb->execfunc ) {
					if( (err=(*x->symb->execfunc)(x->symb, t->text, nargs, argsr, &ld)) == CS_OK )
						stack_push(x->postfixResult, _make_number(ld));
				} else {
					err = CS_NO_SYMBOLS;
				}
				break;
			}
			default: {
				const operator_info_t* oi = _resolve_operator(x, t->type);
				if( ! oi ) {
					err = CS_UNKNOWN_OPERATOR;
					break;
				}
				ASSERT(oi->type == t->type);
				ASSERT(oi->f);
				ASSERT(oi->minArgs);
				if( stack_size(x->postfixResult) < oi->minArgs ) {
					err = CS_BAD_EXPRESSION;
					break;
				}
				number_t args[oi->minArgs];
				for(i=0; i < oi->minArgs; ++i) {
					number_t* pn = (number_t*)stack_pop(x->postfixResult);
					ld = *pn;
					free(pn);
					args[oi->minArgs-i-1] = ld;
					if( (oi->flags & ofInteger) && ! isinteger(ld) )
						err=CS_NOT_INTEGER;
					if( (oi->flags & ofUnsigned) && ld < 0 )
						err=CS_NOT_UNSIGNED;
				}
				if( err==CS_OK && (err=(*oi->f)(x->user, oi->minArgs, args, &ld))==CS_OK )
					stack_push(x->postfixResult, _make_number(ld));
				break;
			}
		}
	}
	TRACE("err=%d", err);
	END_FUNCTION();
	return err;
#undef isinteger
}
static int _evaluate_postfix(calc_context_t* c, number_t* result) {
	int err=CS_OK, i;
	BEGIN_FUNCTION();
	ASSERT(result);
	*result = 0;
	if( ! c->nPostfix )
		return CS_OK;
	ASSERT(c->aPostfix);
	stack_init(&c->postfixResult, free);
	for(i=0; i < c->nPostfix && err==CS_OK; i++)
		err=_cb_evaluate(c->aPostfix[i], c);
	if( err==CS_OK ) {
		if( stack_size(c->postfixResult) == 1 ) {
			number_t* pv = (number_t*)stack_pop(c->postfixResult);
			*result = *pv;
			free(pv);
		} else {
			err = CS_BAD_EXPRESSION;
		}
	}
	stack_deinit(c->postfixResult);
	c->postfixResult = 0;
	TRACE("err=%d", err);
	END_FUNCTION();
	return err;
}

#ifdef _DEBUG_
static int _cb_postfix(void* data, void* cookie) {
	lexer_token_t* t = (lexer_token_t*)data;
	calc_context_t* x = (calc_context_t*)cookie;
	if( t->text )
		buffer_append_va(x->postfixString, "%s ", t->text);
	else if( x->oplookup[t->type] && x->oplookup[t->type]->symbol )
		buffer_append_va(x->postfixString, "%s ", x->oplookup[t->type]->symbol);
	else
		buffer_append_va(x->postfixString, "%c ", t->type);
	return 0;
}
static void _trace_postfix(calc_context_t* c) {
	int i;
	for(i=0; i < c->nPostfix; i++)
		_cb_postfix(c->aPostfix[i], c);
	TRACE("POSTFIX: [ %s ]", buffer_get(c->postfixString));
}
#endif

static void _calc_create(FILE* f, calc_symboltable_t* symb, void* cookie, calc_context_t** ctx) {
	calc_context_t* x = (calc_context_t*)malloc(sizeof(calc_context_t));
	int i;

	x->aPostfix = 0;
	x->nPostfix = 0;
	x->postfixResult = 0;
	x->symb = symb;
	x->user = cookie;

	lexer_create(f, &x->lexer);
	buffer_init(&x->postfixString);

	for(i=0; i < sizeof(x->oplookup)/sizeof(x->oplookup[0]); i++)
		x->oplookup[i] = 0;
	for(i=0; i < sizeof(_opinfo)/sizeof(_opinfo[0]); i++)
		x->oplookup[_opinfo[i].type] = &_opinfo[i];

	*ctx = x;
}
static void _calc_destroy(calc_context_t* ctx) {
	int i;

	for(i=0; i < ctx->nPostfix; i++)
		lexer_free_token(ctx->aPostfix[i]);
	free(ctx->aPostfix);

	if( ctx->postfixResult )
		stack_deinit(ctx->postfixResult);
	buffer_destroy(ctx->postfixString);
	lexer_destroy(ctx->lexer);

	free(ctx);
}

int calc_evaluate(const char* expression, calc_symboltable_t* symb, void* cookie, number_t* result) {
	calc_context_t* c;
	FILE* f;
	int err=CS_OK;

	if( (f=fmemopen((void*)expression, strlen(expression), "r")) == 0 )
		return CS_EMPTY_EXPRESSION;

	_calc_create(f, symb, cookie, &c);

	err=lexer_infix2postfix(c->lexer, _compare_op, c, &c->nPostfix, &c->aPostfix);
	if( err < 0 )
		err = CS_BAD_EXPRESSION;

#ifdef _DEBUG_
	_trace_postfix(c);
#endif

	if( err==CS_OK && (err=_evaluate_postfix(c, result))==CS_OK && symb && symb->setvar )
		(*symb->setvar)(symb, "_", *result);

	_calc_destroy(c);
	fclose(f);

	return err;
}

const char* calc_get_error(int err) {
	switch( err ) {
		case CS_OK:					return "Ok";
		case CS_BAD_EXPRESSION:		return "Bad expression";
		case CS_EMPTY_EXPRESSION:	return "Empty expression";
		case CS_NAN:				return "NaN";
		case CS_INFINITE:			return "Infinite";
		case CS_BAD_NUMBER:			return "Bad number";
		case CS_UNKNOWN_OPERATOR:	return "Unknown operator";
		case CS_BAD_OPERAND:		return "Bad operand";
		case CS_UNKNOWN_FUNCTION:	return "Unknown function";
		case CS_UNKNOWN_VARIABLE:	return "Unknown varlable";
		case CS_NOT_INTEGER:		return "Not an integer";
		case CS_NOT_UNSIGNED:		return "Not an unsigned integer";
		case CS_EXTRA_OPEN_PAREN:	return "Too many openning parenthesis";
		case CS_EXTRA_CLOSE_PAREN:	return "Too manu closing parenthesis";
		case CS_NO_SYMBOLS:			return "No symbol-table";
		case CS_CONSTANT_VAR:		return "Cannot change a constant";
		default:					return "General error";
	}
}

