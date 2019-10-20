#include <stdio.h>

#define INST_HALT	0
#define INST_GOTO_I	1
#define INST_GOTO_R	2
#define INST_GOTO_RI	3
#define INST_MOV_I	4
#define INST_MOV_R	5
#define INST_POP_R	6
#define INST_POP	7
#define INST_PUSH_I	8
#define INST_PUSH_R	9
#define INST_ADD_I	10
#define INST_ADD_R	11
#define INST_LOAD_R	12
#define INST_LOAD_I	13
#define INST_LOAD_RI	14
#define INST_CALL_I	15
#define INST_CALL_R	16
#define INST_RET	17


#define OP_COUNT 18
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
	"ADD_I",
	"ADD_R",
	"LOAD_R",
	"LOAD_I",
	"LOAD_RI",
	"CALL_I",
	"CALL_R",
	"RET"
};

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

Inst make_inst(unsigned int inst, int a, int b, int c){
	Inst i;
	i.inst = inst;
	i.a = a;
	i.b = b;
	i.c = c;
	return i;
}

typedef struct Interp{
	unsigned int reg[12];

	unsigned int stack[512];
	unsigned int stack_length;
} Interp;

void interpret(Interp *interp, Inst *code, unsigned int length, unsigned int start){
	interp->reg[SP] = 0;
	interp->reg[FP] = 0;
	interp->reg[RET] = 0;
	interp->reg[IS] = start;

	interp->stack_length = 512;


	int running = 1;
	while(running){
		if(interp->reg[IS] >= length){
			printf("Cannot do instruction at index %d, the code is only %d long\n", interp->reg[IS], length);
			return;
		}

		Inst i = code[interp->reg[IS]++];
		
		//printf("SP=%d, FP=%d, RET=%d, IS=%d     ", interp->reg[SP], interp->reg[FP], interp->reg[RET], interp->reg[IS]);
		printf("%s %d %d %d\n", OP[i.inst], i.a, i.b, i.c);
	
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
				interp->reg[i.a] = interp->stack[interp->reg[SP]--];
				break;
			case INST_POP:
				interp->reg[SP]--;
				break;
			case INST_PUSH_I:
				interp->stack[++interp->reg[SP]] = i.a;
				break;
			case INST_PUSH_R:
				interp->stack[++interp->reg[SP]] = interp->reg[i.a];
				break;
			case INST_ADD_I:
				interp->reg[i.a] = interp->reg[i.b] + i.c;
				break;
			case INST_ADD_R:
				interp->reg[i.a] = interp->reg[i.b] + interp->reg[i.c];
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
			case INST_CALL_I://Will have to figure out what to do as far as saving the last one
				interp->reg[RET] = interp->reg[IS];
				interp->reg[IS] = i.a;
				break;
			case INST_CALL_R:
				interp->reg[RET] = interp->reg[IS];
				interp->reg[IS] = interp->reg[i.a];
				break;
			case INST_RET:
				interp->reg[IS] = interp->reg[RET];
				break;
			default:
				printf("%d is not an opcode that we understand\n", interp->reg[IS]);
				running = 0;
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
		if(check[j] == '\0') return i;
	}
	return -1;
}


void load_dsm(const char *file_path, Inst *code, int *length){
	FILE* input = fopen(file_path, "r");
	
	char line[256];
	int line_length = 0;

	int line_count = 0;;

	int c = fgetc(input);
	while(c!=EOF){
		if(c == '\n'){
			line[line_length = '\0'];

			char op[64];
			char a[16];
			char b[16];
			char c[16];

			sscanf(line, "%s %s %s %s", op, a, b, c);
			//printf("%s %s %s %s\n", op, a, b, c);			

			int oper = get_op(op);
			if(oper != -1){
				printf("%s\n", OP[oper]);
			}else{
				printf("erm\n");
			}

			line_length = 0;
		}else{
			line[line_length] = c;
			line_length++;
		}
		line_count++;
		c = fgetc(input);
	}
	fclose(input);
}


int main(){
	Inst code[15];
	
	//main:
	code[0] = make_inst(INST_PUSH_R, 9, -1, -1);	//Save the frame pointer on the stack
	code[1] = make_inst(INST_MOV_R, 9, 8, -1);	//Move the stack pointer into the frame pointer
	code[2] = make_inst(INST_PUSH_I, 60, -1, -1);	//Push 60 onto the stack
	code[3] = make_inst(INST_PUSH_I, 9, -1, -1);	//Push 9 onto the stack
	code[4] = make_inst(INST_CALL_I, 7, -1, -1);	//Call the add function 
	code[5] = make_inst(INST_POP_R, 0, -1, -1);	//Pop result of function into register 0
	code[6] = make_inst(INST_HALT, -1, -1, -1);	//Stop the program

	//add:
	code[7] = make_inst(INST_LOAD_RI, 0, 9, 1);	//Pop 9 into register 0
	code[8] = make_inst(INST_LOAD_RI, 1, 9, 2);	//Pop 60 into register 1
	code[9] = make_inst(INST_ADD_R, 0, 0, 1);	//Add register 0 and 1 and store into 0
	code[10] = make_inst(INST_POP, -1, -1, -1);	//Pop 9 off the stack
	code[11] = make_inst(INST_POP, -1, -1, -1);	//Pop 60 off the stack
	code[12] = make_inst(INST_POP_R, 9, -1, -1);	//Pop the original fp back into fp
	code[13] = make_inst(INST_PUSH_R, 0, -1, -1);	//Push the function result onto stack
	code[14] = make_inst(INST_RET, -1, -1, -1);	//Return from function

	Interp interp;
	interpret(&interp, code, 15, 0);

	printf("%d\n", interp.reg[R0]);

	load_dsm("test.dsm", NULL, NULL);

	return 0;
}
