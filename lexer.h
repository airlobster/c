// lexer.h

#ifndef _LEXER_H_
#define _LEXER_H_

#include <stdio.h>

#define	LEXER_EXTENDED	(0x800)
#define	LEXER_OPERAND	(0x400)

#define	IS_LEXER_EXTENDED(t)	(((t) & LEXER_EXTENDED) == LEXER_EXTENDED)
#define	IS_LEXER_OPERAND(t)		(((t) & LEXER_OPERAND) == LEXER_OPERAND)

typedef enum _lexer_tokens_t {
	ttNumber		=LEXER_EXTENDED|LEXER_OPERAND|0,
	ttBinary		=LEXER_EXTENDED|LEXER_OPERAND|1,
	ttOctal			=LEXER_EXTENDED|LEXER_OPERAND|2,
	ttHex			=LEXER_EXTENDED|LEXER_OPERAND|3,
	ttVariable		=LEXER_EXTENDED|LEXER_OPERAND|4,
	ttFunction		=LEXER_EXTENDED|5,
	ttLastFuncArg	=LEXER_EXTENDED|LEXER_OPERAND|6,
	ttUMinus		=LEXER_EXTENDED|7,
	ttUPlus			=LEXER_EXTENDED|8,
	ttPower			=LEXER_EXTENDED|9,
	ttRoot			=LEXER_EXTENDED|10,
	ttSHL			=LEXER_EXTENDED|11,
	ttSHR			=LEXER_EXTENDED|12,
	ttAssign		=LEXER_EXTENDED|13,
} lexer_tokens_t;

typedef struct _lexer_token_t {
	int type;
	char* text;
} lexer_token_t;

typedef int(*lexer_token_compare_t)(const lexer_token_t* t1, const lexer_token_t* t2, void* context, int* ret);

typedef void lexer_t;

void lexer_create(FILE* f, lexer_t** l);
void lexer_destroy(lexer_t* l);
lexer_token_t* lexer_next(lexer_t* l);
void lexer_backtrack(lexer_t* l, lexer_token_t* t);
// lexer_token_t* lexer_make_token(int type, const char* text);
void lexer_free_token(lexer_token_t* t);

int lexer_infix2postfix(lexer_t* l, lexer_token_compare_t compare, void* context, size_t* n, lexer_token_t*** a);

#endif
