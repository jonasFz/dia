#ifndef _H_DSM
#define _H_DSM

#include <stdint.h>

#include "array.h"
//TODO: do we need builder?
#include "builder.h"
#include "scope.h"

//Really need to fix this mess

#define INST_HALT			0
#define INST_GOTO_I			1
#define INST_GOTO_R			2
#define INST_GOTO_RI		3
#define INST_MOV_I			4
#define INST_MOV_R			5
#define INST_POP_R			6
#define INST_POP			7
#define INST_PUSH_I			8
#define INST_PUSH_R			9
#define INST_PUSH_RI		10
#define INST_ADD_I			11
#define INST_ADD_R			12
#define INST_SUB_I			13
#define INST_SUB_R			14
#define INST_MUL_I			15
#define INST_MUL_R			16
#define INST_DIV_I			17
#define INST_DIV_R			18
#define INST_LOAD_R			19
#define INST_LOAD_I			20
#define INST_LOAD_RI		21
#define INST_SAVE_R			22
#define INST_SAVE_I			23
#define INST_SAVE_RI		24
#define INST_CALL_I			25
#define INST_CALL_R			26
#define INST_EXT_CALL_I		27
#define INST_RET			28
#define INST_CMP_I			29
#define INST_JUMP_EQL		30
#define INST_JUMP_NEQL		31
#define INST_NOT			32
#define INST_SWAP_STACK		33

#define OP_COUNT 34

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

#define CMP_E	0
#define CMP_L	1
#define CMP_G	3



typedef struct Inst{
	unsigned int inst;
	int a;
	int b;
	int c;

	int source_line_number;
} Inst;

typedef struct Label{
	char *value;
	int location;
} Label;

typedef struct Interp{
	int32_t reg[12];

	void *stack;
	unsigned int stack_length;

	unsigned int cmp;

	int line;
} Interp;

void push_4(Interp *p, int32_t val);
int32_t pop_4(Interp *p);

typedef struct Code{
	Inst *code;
	Array labels;
	unsigned int length;
	unsigned int cap;

	int current_source_line;
} Code;

void add_label(Code *code, char *label, int value);
int lookup_label(Code *code, char *label);

void interpret(Interp *interp, Code *proc, Scope *scope, unsigned int start);
int load_dsm(const char *file_path, Code *code);
Inst make_inst(unsigned int inst, int a, int b, int c);
void add_inst(Code *code, Inst inst);
Code make_code();
void show_code(Code *code);
void update_line(Code *code, int line);

#endif
