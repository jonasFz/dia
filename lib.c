#include "lib.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXT_COUNT 3

void ext_print(Interp *interp){
	int value = interp->stack[--interp->reg[SP]];
	printf("%d\n", value);
	interp->stack[interp->reg[SP]++] = 0;
}

void ext_get(Interp *interp){
	int c = fgetc(stdin);
	interp->stack[interp->reg[SP]++] = c;
}

void ext_assert(Interp *interp){
	int is_true = interp->stack[--interp->reg[SP]];
	if(!is_true){
		//TODO We really need location information
		printf("Assertion failed!\n");
		printf("Line number %d\n", interp->line + 1);
		exit(0);
	}
	interp->stack[interp->reg[SP]++] = 0;
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

/*
External* get_externals(){
	return externals;
}

void register_externals(Name_Table *nt){
	//TODO figure out what we want to do for functions that aren't used
	//Wont be able to set_location in those cases.
	set_location(nt, "print", 0);
	set_location(nt, "get", 1);
	set_location(nt, "assert", 2);
}*/
