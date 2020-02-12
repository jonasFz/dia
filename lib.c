#include "lib.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXT_COUNT 3

void ext_print(Interp *interp){
	int value = pop_4(interp);
	printf("%d\n", value);
	push_4(interp, 0);
}

void ext_get(Interp *interp){
	int c = fgetc(stdin);
	push_4(interp, c);
}

void ext_assert(Interp *interp){
	int is_true = pop_4(interp);
	if(!is_true){
		//TODO We really need location information
		printf("Assertion failed!\n");
		printf("Line number %d\n", interp->line + 1);
		exit(0);
	}
	push_4(interp, 0);
}

//All of these function currently MUST leave something on the stack
External externals[EXT_COUNT] = {
	ext_print,
	ext_get,
	ext_assert
};

const char *names[EXT_COUNT] = {
	"print",
	"get",
	"assert"
};

External get_external_by_index(unsigned int index){
	assert(index < EXT_COUNT);
	return externals[index];
}

int lookup_external_index(const char *name){
	for (int i = 0; i < EXT_COUNT; i++){
		if(strcmp(name, names[i]) == 0){
			return i;
		}
	}
	return -1;
}
