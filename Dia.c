#include "Lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
}
