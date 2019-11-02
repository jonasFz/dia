#include "lib.h"

#include <stdio.h>
#include <stdlib.h>


void ext_print(Interp *interp){
	int value = interp->stack[--interp->reg[SP]];
	printf("%d\n", value);
}

External externals[EXT_COUNT] = {
	ext_print
};

External* get_externals(){
	return externals;
}

void register_externals(Name_Table *nt){
	//TODO figure out what we want to do for functions that aren't used
	//Wont be able to set_location in those cases.
	set_location(nt, "print", 0);
}
