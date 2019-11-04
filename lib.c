#include "lib.h"

#include <stdio.h>
#include <stdlib.h>

void ext_print(Interp *interp){
	int value = interp->stack[--interp->reg[SP]];
	printf("%d\n", value);
	interp->stack[interp->reg[SP]++] = 0;
}

void ext_get(Interp *interp){
	int c = fgetc(stdin);
	interp->stack[interp->reg[SP]++] = c;
}

//All of these function currently MUST leave something on the stack
External externals[EXT_COUNT] = {
	ext_print,
	ext_get
};

External* get_externals(){
	return externals;
}

void register_externals(Name_Table *nt){
	//TODO figure out what we want to do for functions that aren't used
	//Wont be able to set_location in those cases.
	set_location(nt, "print", 0);
	set_location(nt, "get", 1);
}
