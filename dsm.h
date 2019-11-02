#ifndef _H_DSM
#define _H_DSM

#include "array.h"
#include "builder.h"

#define INST_HALT		0
#define INST_GOTO_I		1
#define INST_GOTO_R		2
#define INST_GOTO_RI	3
#define INST_MOV_I		4
#define INST_MOV_R		5
#define INST_POP_R		6
#define INST_POP		7
#define INST_PUSH_I		8
#define INST_PUSH_R		9
#define INST_ADD_I		10
#define INST_ADD_R		11
#define INST_SUB_I		12
#define INST_SUB_R		13
#define INST_MUL_I		14
#define INST_MUL_R		15
#define INST_DIV_I		16
#define INST_DIV_R		17
#define INST_LOAD_R		18
#define INST_LOAD_I		19
#define INST_LOAD_RI	20
#define INST_SAVE_R		21
#define INST_SAVE_I		22
#define INST_SAVE_RI	23
#define INST_CALL_I		24
#define INST_CALL_R		25
#define INST_RET		26


#define OP_COUNT 27

#define R0	0
#define R1	1
#define R2	2
#define R3	3
#define R4	4
#define R5	5
#define R6	6
#define R7	7
#define SP	8
#define FP	9
#define RET	10
#define IS	11

typedef struct Inst{
	unsigned int inst;
	int a;
	int b;
	int c;
} Inst;

typedef struct Label{
	char *value;
	int location;
} Label;

typedef struct Interp{
	unsigned int reg[12];

	unsigned int stack[512];
	unsigned int stack_length;
} Interp;

typedef struct Code{
	Inst *code;
	Array labels;
	unsigned int length;
	unsigned int cap;
} Code;

void add_label(Code *code, char *label, int value);
int lookup_label(Code *code, char *label);

void interpret(Interp *interp, Code *proc, Name_Table *nt, unsigned int start);
int load_dsm(const char *file_path, Code *code);
Inst make_inst(unsigned int inst, int a, int b, int c);
void add_inst(Code *code, Inst inst);
Code make_code();
void show_code(Code *code);

#endif
