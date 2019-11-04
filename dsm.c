#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dsm.h"
#include "lib.h"

const char *OP[OP_COUNT] ={
	"HALT",
	"GOTO_I",
	"GOTO_R",
	"GOTO_RI",
	"MOV_I",
	"MOV_R",
	"POP_R",
	"POP",
	"PUSH_I",
	"PUSH_R",
	"PUSH_RI",
	"ADD_I",
	"ADD_R",
	"SUB_I",
	"SUB_R",
	"MUL_I",
	"MUL_R",
	"DIV_I",
	"DIV_R",
	"LOAD_R",
	"LOAD_I",
	"LOAD_RI",
	"SAVE_R",
	"SAVE_I",
	"SAVE_RI",
	"CALL_I",
	"CALL_R",
	"CALL_EXT_I",
	"RET",
	"CMP_I",
	"JUMP_EQL",
	"JUMP_NEQL",
	"NOT",
	"SWP_STACK"
};

Inst make_inst(unsigned int inst, int a, int b, int c){
	Inst i;
	i.inst = inst;
	i.a = a;
	i.b = b;
	i.c = c;
	return i;
}

Code make_code(){
	Code c;
	c.code = (Inst *)malloc(sizeof(Inst)*128);
	c.length = 0;
	c.cap = 128;

	c.labels = make_array(sizeof(Label));

	return c;
}

void add_inst(Code *code, Inst inst){
	if(code->length == code->cap){
		code->code = (Inst *)realloc(code->code, sizeof(Inst) * code->cap*2);
		code->cap *= 2;
	}
	code->code[code->length++] = inst;
}

void show_code(Code *code){
	for (int i = 0; i < code->length; i++){
		Inst inst = code->code[i];
		printf("%3d %s %d %d %d\n",i, OP[inst.inst], inst.a, inst.b, inst.c);
	}
}

void interpret(Interp *interp, Code *proc, Name_Table *nt, unsigned int start){
	register_externals(nt);
	External *externals = get_externals();

	Inst *code = proc->code;
	unsigned int length = proc->length;

	interp->reg[SP] = 0;
	interp->reg[FP] = 0;
	interp->reg[RET] = 0;
	interp->reg[IS] = start;

	interp->stack_length = 512;

	for(int i = 0; i < interp->stack_length; i++){
		interp->stack[i] = 0;
	}


	int running = 1;
	while(running){
		if(interp->reg[IS] >= length){
			printf("Cannot do instruction at index %d, the code is only %d long\n", interp->reg[IS], length);
			return;
		}

		Inst i = code[interp->reg[IS]++];
		//printf("SP=%d, FP=%d, RET=%d, IS=%d\n", interp->reg[SP], interp->reg[FP], interp->reg[RET], interp->reg[IS]);
		//printf("%2d: %10s %2d %2d %2d :: ",interp->reg[IS]-1, OP[i.inst], i.a, i.b, i.c);

		switch(i.inst){
			case INST_HALT:
				running = 0;
				break;
			case INST_GOTO_I:
				interp->reg[IS] = i.a;
				break;
			case INST_GOTO_R:
				interp->reg[IS] = interp->reg[i.a];
				break;
			case INST_GOTO_RI:
				interp->reg[IS] = interp->reg[i.a] + i.b;
				break;
			case INST_MOV_I:
				interp->reg[i.a] = i.b;
				break;
			case INST_MOV_R:
				interp->reg[i.a] = interp->reg[i.b];
				break;
			case INST_POP_R:
				interp->reg[i.a] = interp->stack[--interp->reg[SP]];
				break;
			case INST_POP:
				interp->reg[SP]--;
				break;
			case INST_PUSH_I:
				interp->stack[interp->reg[SP]++] = i.a;
				break;
			case INST_PUSH_R:
				interp->stack[interp->reg[SP]++] = interp->reg[i.a];
				break;
			case INST_ADD_I:
				interp->reg[i.a] = interp->reg[i.b] + i.c;
				break;
			case INST_ADD_R:
				interp->reg[i.a] = interp->reg[i.b] + interp->reg[i.c];
				break;
			case INST_SUB_I:
				interp->reg[i.a] = interp->reg[i.b] - i.c;
				break;
			case INST_SUB_R:
				interp->reg[i.a] = interp->reg[i.b] - interp->reg[i.c];
				break;
			case INST_MUL_I:
				interp->reg[i.a] = interp->reg[i.b] * i.c;
				break;
			case INST_MUL_R:
				interp->reg[i.a] = interp->reg[i.b] * interp->reg[i.c];
				break;
			case INST_DIV_I:
				interp->reg[i.a] = interp->reg[i.b] / i.c;
				break;
			case INST_DIV_R:
				interp->reg[i.a] = interp->reg[i.b] / interp->reg[i.c];
				break;
			case INST_LOAD_R://Just does stack for the moment
				interp->reg[i.a] = interp->stack[interp->reg[i.b]];
				break;
			case INST_LOAD_I:
				interp->reg[i.a] = interp->stack[i.b];
				break;
			case INST_LOAD_RI:
				interp->reg[i.a] = interp->stack[interp->reg[i.b] + i.c];
				break;
			case INST_SAVE_R:
				interp->stack[interp->reg[i.a]] = interp->reg[i.b];
				break;
			case INST_SAVE_I:
				interp->stack[interp->reg[i.a]] = i.b;
				break;
			case INST_SAVE_RI:
				printf("SAVE_RI NOT IMPLEMENTED");
				break;
			case INST_CALL_I:
				interp->reg[RET] = interp->reg[IS];
				interp->reg[IS] = ((Row *)get_item(&nt->names, i.a))->location; 
				break;
			case INST_CALL_R:
				interp->reg[RET] = interp->reg[IS];
				interp->reg[IS] = interp->reg[i.a];
				break;
			case INST_EXT_CALL_I:
				externals[((Row *)get_item(&nt->names, i.a))->location](interp);
				break;
			case INST_RET:
				interp->reg[IS] = interp->reg[RET];
				break;
			case INST_CMP_I:
				if(interp->reg[i.a] == i.b){
					interp->cmp = CMP_E;
				}else if(interp->reg[i.a] < i.b){
					interp->cmp = CMP_L;
				}else{
					interp->cmp = CMP_G;
				}
				break;
			case INST_JUMP_EQL:
				if(interp->cmp == CMP_E){
					interp->reg[IS] = i.a;
				}
				break;
			case INST_JUMP_NEQL:
				if(interp->cmp != CMP_E){
					interp->reg[IS] = i.a;
				}
				break;
			case INST_NOT:
				interp->reg[i.a] = !interp->reg[i.a];
				break;
			case INST_SWAP_STACK:
				interp->stack[interp->reg[SP]] = interp->stack[interp->reg[SP]-2];
				interp->stack[interp->reg[SP]-2] = interp->stack[interp->reg[SP]-1];
				interp->stack[interp->reg[SP]-1] = interp->stack[interp->reg[SP]];
				break;
			default:
				printf("%d is not an opcode that we understand\n", interp->reg[IS]);
				running = 0;
		}
		
		if(interp->reg[SP] <0 || interp->reg[SP] >= interp->stack_length){
			printf("Bad stack value, %d is not in the range %d to %d\n", interp->reg[SP], 0, interp->stack_length);
			exit(1);
		}
/*		printf("(R0=%2d, R1=%2d, R2=%2d, RET=%2d) ", interp->reg[R0], interp->reg[R1], interp->reg[R2], interp->reg[RET]);
		for(int i = 0; i < interp->reg[SP]; i++){
			printf("%d-", interp->stack[i]);
		}
		printf("\n");

		fgetc(stdin);
*/
		if((interp->reg[IS]) >= proc->length){		
			printf("Instruction location %d is out of range of this code\n", interp->reg[IS]+1);
		}
	}
}

int get_op(char* line){
	for(int i = 0;i < OP_COUNT; i++){
		const char *check = OP[i];

		int j = 0;
		while(check[j] != '\0'){
			if(check[j] != line[j]){
				break;
			}
			j++;
		}
		if(check[j] == '\0' && line[j] == '\0') return i;
	}
	return -1;
}


int load_dsm(const char *file_path, Code *code){
	FILE* input = fopen(file_path, "r");
	if(input == NULL){
		return 0;
	}

	char line[256];
	int line_length = 0;

	int line_count = 0;

	int cur = fgetc(input);
	while(cur!=EOF){
		if(cur != '\n'){
			line[line_length++] = cur;
			cur = fgetc(input);
			continue;
		}
		
		line[line_length] = '\0';

		if(line[0] == '\0'){
			line_length = 0;
			cur = fgetc(input);
			continue;
		}

		char op[64];
		int av = -1;
		int bv = -1;
		int cv = -1;

		sscanf(line, "%s %d %d %d", op, &av, &bv, &cv);

		int oper = get_op(op);

		if(oper != -1){
			add_inst(code, make_inst(oper, av, bv, cv));
			printf("%s %d %d %d\n", OP[oper], av, bv, cv);
		}else{
			printf("%s\n", line);
		}

		line_length = 0;
		line_count++;

		cur = fgetc(input);
	}
	fclose(input);
	return 1;
}

