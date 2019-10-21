#include <stdio.h>
#include <stdlib.h>

#include "dsm.h"

int main(int argc, char **argv){

	if( argc != 2){
		printf("Please specify a file to load\n");
		return 0;
	}

	Code proc = make_code();

	if(!load_dsm(argv[1], &proc)){
		printf("Failed to load file '%s'\n", argv[1]);
		return 0;
	}


	printf("%d\n", proc.length);

	Interp interp;
	interpret(&interp, &proc, 0);

	printf("%d\n", interp.reg[R0]);

	return 0;
}
