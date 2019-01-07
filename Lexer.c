#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Lexer.h"

/*
#define TOKEN_IDENT 		0
#define TOKEN_ASSIGNMENT	1
#define TOKEN_INT		2

#define TOKENIZER_STATE_BEGIN	0
#define TOKENIZER_STATE_HALT	1
#define TOKENIZER_STATE_IDENT	2
#define TOKENIZER_STATE_ERROR	3
#define TOKENIZER_STATE_OPER	4
#define TOKENIZER_STATE_INT	5

unsigned int number_of_tokens = 0;

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
*/

unsigned int number_of_tokens = 0;

int make_tokenizer(Tokenizer* tokenizer, char* data){
	tokenizer->data = data;
	tokenizer->index = 0;
	tokenizer->end = 0;
	tokenizer->state = TOKENIZER_STATE_BEGIN;
	return 1;
}

char current_character(Tokenizer* tokenizer){
	return tokenizer->data[tokenizer->index];
}
char next_character(Tokenizer* tokenizer){
	char character = tokenizer->data[++tokenizer->index];
	if (character == '\0'){
		tokenizer->end = 1;
		tokenizer->index--;
	}
	return character;
}

int is_ident_character(char c){
	return (c>='a' && c<='z') || (c>='A'&&c<='Z');
}

int is_space_character(char c){
	return c == ' ' || c == '\n' || c == '\0';
}

int is_operator_character(char c){
	return c == '=';
}

int is_digit_character(char c){
	return c >='0' && c<='9';
}

int accept_token(char* buffer, int length, Token* token){
	token->value = (char*)malloc(sizeof(char)*(length+1));
	memcpy(token->value, buffer, length);
	token->value[length] = '\0';
	token->token_number = number_of_tokens++;
	return 1;
}

int next_token(Tokenizer* tokenizer,Token* token){
	char value_buffer[512];
	int buffer_length = 0;
	
	char current = current_character(tokenizer);
	
	while(tokenizer->state!=TOKENIZER_STATE_HALT){
		switch(tokenizer->state){
			case TOKENIZER_STATE_BEGIN:
				if (is_ident_character(current)){
					tokenizer->state = TOKENIZER_STATE_IDENT;
				}else if(is_operator_character(current)){
					tokenizer->state = TOKENIZER_STATE_OPER;
				}else if(is_digit_character(current)){
					tokenizer->state = TOKENIZER_STATE_INT;
				}else if(current == EOF || current == '\0'){
					tokenizer->state = TOKENIZER_STATE_HALT;
				}else if(is_space_character(current)){
					current = next_character(tokenizer);
				}else{
					printf("Unsure what to do with token starting with '%c' %d\n",current,(int)current);
					tokenizer->state = TOKENIZER_STATE_ERROR;
				}
				break;
			case TOKENIZER_STATE_IDENT:
				if(is_ident_character(current)){
					value_buffer[buffer_length++] = current;
					current = next_character(tokenizer);					
				}else if (current == '\0'){
					//This could be removed and treated as a space character but one thing at a time.
					// But maybe not? I'll have to think about that.
					tokenizer->state = TOKENIZER_STATE_HALT;
				}else if(is_space_character(current)){
					tokenizer->state = TOKENIZER_STATE_BEGIN;
					accept_token(value_buffer, buffer_length, token);
					buffer_length = 0;
					return 1;
				}else{
					printf("Unsure what to do with the character '%c' when reading an IDENTIFIER\n",current);
					tokenizer->state = TOKENIZER_STATE_ERROR;
				}
				break;
			case TOKENIZER_STATE_OPER:
				if(current == '='){
					tokenizer->state = TOKENIZER_STATE_BEGIN;
					value_buffer[buffer_length++] = current;
					current = next_character(tokenizer);
					accept_token(value_buffer, buffer_length, token);
					buffer_length = 0;
					return 1;
				}else{
					printf("%c is not an operator character, should not even be in the state TOKENIZER_STATE_OPER\n",current);
					tokenizer->state = TOKENIZER_STATE_ERROR;
				}
				break;
			case TOKENIZER_STATE_INT:
				if(is_digit_character(current)){
					value_buffer[buffer_length++] = current;
					current = next_character(tokenizer);
				}else if(is_space_character(current)){
					tokenizer->state = TOKENIZER_STATE_BEGIN;
					accept_token(value_buffer, buffer_length, token);
					buffer_length = 0;
					current = next_character(tokenizer);
					return 1;
				}else{
					printf("Unsure what to do with the character '%c' when reading in state TOKENIZER_STATE_INT\n", current);
					tokenizer->state = TOKENIZER_STATE_ERROR;	
				}
				break;
			case TOKENIZER_STATE_ERROR:
				return 0;
				break;
			case TOKENIZER_STATE_HALT:
				return 0;
				break;
			default:
				printf("%d is not a valid state for the function 'next_token'\nWe should NOT be here\n",tokenizer->state);
				return 0;
		}
	}

	return 0;
}
/*
char* load_file(const char* file_path){
	FILE* in_file = fopen(file_path,"r");
	if (in_file == NULL) return NULL;
	fseek(in_file,0,SEEK_END);
	
	int size = ftell(in_file);
	fseek(in_file,0,SEEK_SET);
	
	char* result = (char *)malloc(sizeof(char)*(size+1));

	char read = fgetc(in_file);
	int position = 0;
	while(read != EOF){
		result[position] = read;
		position++;
		read = fgetc(in_file);
	}
	result[position] = '\0';
	
	return result;
}
*/
/*
int main(int argc, char** argv){
	printf("Hello\n");

	char* file = load_file("test.txt");
	printf("%s\n",file);

	Tokenizer tokenizer;
	make_tokenizer(&tokenizer,file);

	Token token;
	while(tokenizer.state != TOKENIZER_STATE_HALT && tokenizer.state != TOKENIZER_STATE_ERROR){
		if(!next_token(&tokenizer,&token)){
			break;		
		}
		printf("Token(%s)#%d\n",token.value,token.token_number);
	}

	return 0;
}*/
