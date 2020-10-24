// lexer.c

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "a_debug.h"
#include "dlist.h"
#include "fstack.h"
#include "buffer.h"
#include "lexer.h"

typedef enum _lexer_states_t {
	ltDetect,
	ltNumberCheckBase,
	ltNumberDigits,
	ltNumberBinDigits,
	ltNumberOctDigits,
	ltNumberHexDigits,
	ltNumberFraction,
	ltNumberExponent,
	ltNumberExponentSign,
	ltNumberComplete10,
	ltNumberComplete16,
	ltNumberComplete8,
	ltNumberComplete2,
	ltName,
	ltNameComplete,
	ltBeginFuncParams,
	ltOperatorMulOrPow,
	ltOperatorDivOrRoot,
	ltOperatorSHL,
	ltOperatorSHR,
	ltLiteral,
} lexer_states_t;

typedef struct _lexer_context_t {
	FILE* f;
	buffer_t* buffer;
	fstack_t* state;
	fstack_t* backtrack;
	int last;
} lexer_context_t;


void lexer_create(FILE* f, lexer_t** l) {
	lexer_context_t* x = (lexer_context_t*)malloc(sizeof(lexer_context_t));
	x->f = f;
	buffer_init(&x->buffer);
	stack_init(&x->state, 0);
	stack_init(&x->backtrack, free);
	stack_push(x->state, (void*)ltDetect);
	x->last = 0;
	*l = x;
}

void lexer_destroy(lexer_t* l) {
	lexer_context_t* x = (lexer_context_t*)l;
	buffer_destroy(x->buffer);
	stack_deinit(x->state);
	stack_deinit(x->backtrack);
	free(x);
}

lexer_token_t* lexer_make_token(int type, const char* text) {
	lexer_token_t* t = (lexer_token_t*)malloc(sizeof(lexer_token_t));
	t->type = type;
	t->text = text ? strdup(text) : 0;
	return t;
}

static int _lastWasOperand(lexer_context_t* l) {
	return IS_LEXER_OPERAND(l->last) || l->last==')';
}

lexer_token_t* lexer_next(lexer_t* l) {
#define push_state(st)	stack_push(x->state, (void*)(st))
#define pop_state()		((lexer_states_t)(long)stack_pop(x->state))
#define unget(c)		ungetc((c), x->f)
#define	text()			buffer_get(x->buffer)
#define append(c)		buffer_append(x->buffer, (c))
	lexer_context_t* x = (lexer_context_t*)l;
	lexer_token_t* t = 0;
	if( ! stack_empty(x->backtrack) )
		t = (lexer_token_t*)stack_pop(x->backtrack);
	while( ! t ) {
		if( stack_empty(x->state) )
			break;
		lexer_states_t st = (lexer_states_t)(long)stack_peek(x->state);
		int c = fgetc(x->f);
		switch( st ) {
			case ltDetect: {
				if( isspace(c) )
					break;
				buffer_reset(x->buffer);
				if( c == EOF )
					pop_state();
				else if( c == '0' ) {
					push_state(ltNumberCheckBase);
				} else if( isdigit(c) || c=='.' ) {
					push_state(ltNumberComplete10);
					push_state(ltNumberExponent);
					push_state(ltNumberFraction);
					push_state(ltNumberDigits);
					unget(c);
				} else if( isalpha(c) || c=='_' ) {
					push_state(ltNameComplete);
					push_state(ltName);
					unget(c);
				} else if( c == '*' ) {
					push_state(ltOperatorMulOrPow);
				} else if( c == '/' ) {
					push_state(ltOperatorDivOrRoot);
				} else if( c == '<' ) {
					push_state(ltOperatorSHL);
				} else if( c == '>' ) {
					push_state(ltOperatorSHR);
				} else if( c == '-' ) {
					t=lexer_make_token(_lastWasOperand(x) ? c : ttUMinus, 0);
				} else if( c == '+' ) {
					t=lexer_make_token(_lastWasOperand(x) ? c : ttUPlus, 0);
				} else if( c == ',' ) {
					// ignore
				} else {
					t=lexer_make_token(c, 0);
				}
				break;
			}
			case ltNumberCheckBase: {
				pop_state();
				if(  c == 'b' ) {
					push_state(ltNumberComplete2);
					push_state(ltNumberBinDigits);
				} else if( c == 'x' ) {
					append('0');
					append('x');
					push_state(ltNumberComplete16);
					push_state(ltNumberHexDigits);
				} else if( isdigit(c) ) {
					unget(c);
					push_state(ltNumberComplete8);
					push_state(ltNumberOctDigits);
				} else if( c == '.' ) {
					append('0');
					unget(c);
					push_state(ltNumberComplete10);
					push_state(ltNumberExponent);
					push_state(ltNumberFraction);
				} else {
					unget(c);
					t=lexer_make_token(ttNumber, "0");
				}
				break;
			}
			case ltNumberBinDigits: {
				if( c=='1' || c=='0' ) {
					append(c);
				} else {
					unget(c);
					pop_state();
				}
				break;
			}
			case ltNumberOctDigits: {
				if( c>='0' && c<='7' ) {
					append(c);
				} else {
					unget(c);
					pop_state();
				}
				break;
			}
			case ltNumberHexDigits: {
				if( isxdigit(c) ) {
					append(c);
				} else {
					unget(c);
					pop_state();
				}
				break;
			}
			case ltOperatorMulOrPow: {
				pop_state();
				if( c == '*' ) {
					t=lexer_make_token(ttPower, 0);
				} else {
					unget(c);
					t=lexer_make_token('*', 0);
				}
				break;
			}
			case ltOperatorDivOrRoot: {
				pop_state();
				if( c == '/' ) {
					t=lexer_make_token(ttRoot, 0);
				} else {
					unget(c);
					t=lexer_make_token('/', 0);
				}
				break;
			}
			case ltOperatorSHL: {
				pop_state();
				if( c == '<' ) {
					t=lexer_make_token(ttSHL, 0);
				} else {
					unget(c);
				}
				break;
			}
			case ltOperatorSHR: {
				pop_state();
				if( c == '>' ) {
					t=lexer_make_token(ttSHR, 0);
				} else {
					unget(c);
				}
				break;
			}
			case ltNumberDigits: {
				if( isdigit(c) ) {
					append(c);
				} else {
					unget(c);
					pop_state();
				}
				break;
			}
			case ltNumberFraction: {
				pop_state();
				if( c == '.' ) {
					append(c);
					push_state(ltNumberDigits);
				} else {
					unget(c);
				}
				break;
			}
			case ltNumberExponent: {
				pop_state();
				if( c == 'E' || c == 'e' ) {
					append(c);
					push_state(ltNumberFraction);
					push_state(ltNumberDigits);
					push_state(ltNumberExponentSign);
				} else {
					unget(c);
				}
				break;
			}
			case ltNumberExponentSign: {
				pop_state();
				if( c=='-' || c=='+' )
					append(c);
				else
					unget(c);
				break;
			}
			case ltNumberComplete10: {
				pop_state();
				unget(c);
				t = lexer_make_token(ttNumber, text());
				break;
			}
			case ltNumberComplete16: {
				pop_state();
				unget(c);
				t = lexer_make_token(ttHex, text());
				break;
			}
			case ltNumberComplete8: {
				pop_state();
				unget(c);
				t = lexer_make_token(ttOctal, text());
				break;
			}
			case ltNumberComplete2: {
				pop_state();
				unget(c);
				t = lexer_make_token(ttBinary, text());
				break;
			}
			case ltName: {
				if( isalnum(c) || c=='_' ) {
					append(c);
				} else {
					unget(c);
					pop_state();
				}
				break;
			}
			case ltNameComplete: {
				if( isspace(c) )
					break;
				pop_state();
				if( c == '(' ) {
					// name is a function identifier
					unget(c);
					t = lexer_make_token(ttFunction, text());
					push_state(ltBeginFuncParams);
				} else if( c == '=' ) {
					// assigning to a variable
					t = lexer_make_token(ttAssign, text());
				} else {
					// de-referencing a variable
					unget(c);
					t = lexer_make_token(ttVariable, text());
				}
				break;
			}
			case ltBeginFuncParams: {
				pop_state();
				unget(c);
				t = lexer_make_token(ttLastFuncArg, 0);
				break;
			}
			default: {
				ASSERT(0);
			}
		}
	}
	if( t )
		x->last = t->type;
	return t;
#undef push_state
#undef pop_state
#undef unget
#undef text
#undef append
}

void lexer_backtrack(lexer_t* l, lexer_token_t* t) {
	lexer_context_t* x = (lexer_context_t*)l;
	ASSERT(t);
	stack_push(x->backtrack, t);
}

void lexer_free_token(lexer_token_t* t) {
	if( t->text )
		free(t->text);
	free(t);
}

/*
	Read an infix expression by the given lexer, and output an array
	of lexer tokens ordered as a postfix expression.
*/
int lexer_infix2postfix(lexer_t* l, lexer_token_compare_t compare, void* context,
		size_t* n, lexer_token_t*** a) {
	BEGIN_FUNCTION();
	dlist_t* out;
	fstack_t* operators;
	int err=0;

	ASSERT(l);
	ASSERT(compare);
	ASSERT(n);
	ASSERT(a);

	stack_init(&operators, 0);
	dlist_init(0, &out);

	while( ! err ) {
		lexer_token_t* t = lexer_next(l);
		if( ! t )
			break;
		if( IS_LEXER_OPERAND(t->type) ) {
			dlist_push_back(out, t);
		} else if( t->type == '(' ) {
			stack_push(operators, t);
		} else if( t->type == ')' ) {
			int found=0;
			lexer_free_token(t); // never used as an operator
			while( ! stack_empty(operators) ) {
				lexer_token_t* ot = (lexer_token_t*)stack_pop(operators);
				if( ot->type == '(' ) {
					found=1;
					lexer_free_token(ot);
					break;
				}
				dlist_push_back(out, ot);
			}
			if( ! found )
				err=-1;
		} else {
			while( !err && ! stack_empty(operators) ) {
				lexer_token_t* prevt = (lexer_token_t*)stack_peek(operators);
				if( prevt->type == '(' )
					break;
				// compare precendence
				int cmp = 0;
				err = (*compare)(t, prevt, context, &cmp);
				if( cmp > 0 )
					break;
				dlist_push_back(out, prevt);
				stack_pop(operators);
			}
			stack_push(operators, t);
		}
	}

	// handle remaining operators
	while( ! stack_empty(operators) ) {
		lexer_token_t* t = (lexer_token_t*)stack_pop(operators);
		dlist_push_back(out, t);
	}

	dlist_detach(out, n, (void***)a);

	dlist_deinit(out);
	stack_deinit(operators);

	END_FUNCTION();
	return err;
}

