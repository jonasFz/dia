#include <stdlib.h>
#include <string.h>

#include "bc.h"

bytecode make_bytecode(op op, s32 r0, s32 r1, s32 r2, s32 r3){
	bytecode bc;
	bc.r0.s32 = r0;
	bc.r1.s32 = r1;
	bc.r2.s32 = r2;
	bc.r3.s32 = r3;

	return bc;
}

bc_code make_bc_code(){
	bc_code bcode;
	bcode.code = (bytecode *)malloc(sizeof(bytecode) * 256);
	bcode.capacity = 256;
	bcode.length = 0;

	return bcode;
}

bc_code join(bc_code *a, bc_code *b){
	bc_code bcode;
	bcode.code = (bytecode *)malloc(sizeof(bytecode)*(a->length + b->length));
	bcode.capacity = a->length + b->length;
	memcpy(bcode.code, a->code, a->length);
	memcpy(bcode.code + a->length, b->code, b->length);

	return bcode;
}

void add_bytecode(bc_code *bcode, bytecode bc){
	if(bcode->length == bcode->capacity){
		bcode->code = (bytecode *)realloc(bcode->code, sizeof(bytecode) * bcode->capacity * 2);
		bcode->capacity *= 2;
	}
	bcode->code[bcode->length++] = bc;
}
