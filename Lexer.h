#ifndef LEXER_H
#define LEXER_H

#define TOKEN_IDENT 		0
#define TOKEN_ASSIGNMENT	1
#define TOKEN_INT		2

#define TOKENIZER_STATE_BEGIN	0
#define TOKENIZER_STATE_HALT	1
#define TOKENIZER_STATE_IDENT	2
#define TOKENIZER_STATE_ERROR	3
#define TOKENIZER_STATE_OPER	4
#define TOKENIZER_STATE_INT	5

typedef struct Token{
	unsigned int token_number;
	unsigned int type;
	char* value;
} Token;

typedef struct Tokenizer{
	char* data;
	unsigned int index;

	unsigned int end;

	unsigned int state;
} Tokenizer;

int make_tokenizer(Tokenizer* tokenizer, char* data);

int next_token(Tokenizer* tokenizer,Token* token);

#endif
