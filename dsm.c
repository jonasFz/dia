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

void push_4(Interp *p, int32_t val){
	*((int32_t *)(p->stack+p->reg[SP])) = val;
	p->reg[SP] += 4;
}

int32_t pop_4(Interp *p){
	p->reg[SP] -= 4;
	return *((int32_t *)(p->stack+p->reg[SP]));
}

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

	c.current_source_line = 0;

	return c;
}

void update_line(Code *code, int line){
	code->current_source_line = line;
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
void allocate_stack(Interp *interp, unsigned int stack_length){
	interp->stack = malloc(stack_length);
	if(interp->stack == NULL){
		printf("Failed to allocate stack space for the interpreter\n");
		exit(1);
	}
	memset(interp->stack, 0, stack_length);
	interp->stack_length = stack_length;
}
void interpret(Interp *interp, Code *proc, Scope *scope, unsigned int start){

	Inst *code = proc->code;
	unsigned int length = proc->length;

	interp->reg[SP] = 0;
	interp->reg[FP] = 0;
	interp->reg[RET] = 0;
	interp->reg[IS] = start;


	allocate_stack(interp, 2048);

	int running = 1;
	while(running){
		if(interp->reg[IS] >= length){
			printf("Cannot do instruction at index %d, the code is only %d long\n", interp->reg[IS], interp->stack_length);
			return;
		}

		Inst i = code[interp->reg[IS]++];
		interp->line = i.source_line_number;
		//printf("Line number = %d\n", i.source_line_number);
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
				interp->reg[i.a] = pop_4(interp);
				break;
			case INST_POP:
				interp->reg[SP]-=4;
				break;
			case INST_PUSH_I:
				push_4(interp, i.a);
				break;
			case INST_PUSH_R:
				push_4(interp, interp->reg[i.a]);
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
				memcpy(&interp->reg[i.a], interp->stack+interp->reg[i.b], 4);
				break;
			case INST_LOAD_I:
				memcpy(&interp->reg[i.a], interp->stack+i.b, 4);
				break;
			case INST_LOAD_RI:
				memcpy(&interp->reg[i.a], interp->stack+interp->reg[i.b] + i.c, 4);
				break;
			case INST_SAVE_R:
				memcpy(interp->stack+interp->reg[i.a], &interp->reg[i.b], 4);
				break;
			case INST_SAVE_I:
				memcpy(interp->stack+interp->reg[i.a], &i.b, 4);
				break;
			case INST_SAVE_RI:
				printf("SAVE_RI NOT IMPLEMENTED");
				break;
			case INST_CALL_I:
				interp->reg[RET] = interp->reg[IS];
				interp->reg[IS] = get_by_index(scope, i.a)->location;
				break;
			case INST_CALL_R:
				interp->reg[RET] = interp->reg[IS];
				interp->reg[IS] = interp->reg[i.a];
				break;
			case INST_EXT_CALL_I:
				get_external_by_index(i.a)(interp);
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
				memcpy(interp->stack+interp->reg[SP], 	interp->stack+interp->reg[SP]-8, 4);
				memcpy(interp->stack+interp->reg[SP]-8, interp->stack+interp->reg[SP]-4, 4);
				memcpy(interp->stack+interp->reg[SP]-4, interp->stack+interp->reg[SP],   4);
				break;
			default:
				printf("%d is not an opcode that we understand\n", interp->reg[IS]);
				running = 0;
		}
		
		if(interp->reg[SP] <0 || interp->reg[SP] >= length){
			printf("Bad stack value, %d is not in the range %d to %d\n", interp->reg[SP], 0, interp->stack_length);
			exit(1);
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

