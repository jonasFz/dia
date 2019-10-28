#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "type.h"
#include "dsm/dsm.h"
#include "array.h"

void verify(Type_Table * tt, Scope *scope, Node *n){
	if(n->type == TYPE_GLOBAL){
		n->scope = scope;
		Array_Iter at = make_array_iter(&n->nodes);
		Node *node = NULL;
		while((node = (Node *)next_item(&at)) != NULL){
			verify(tt, scope, node);
		}
	}else if(n->type == TYPE_FUNCTION){
		//Not sure if this is the scope that makes most sense, doesn't really need anyscope
		n->scope = scope;
		Node *block = (Node *)INDEX(n->nodes, 1);;
		verify(tt, n->scope, block);

	}else if(n->type == TYPE_BLOCK){
		n->scope = create_scope(scope);
		Array_Iter at = make_array_iter(&n->nodes);
		Node *node = NULL;
		while((node = (Node *)next_item(&at)) != NULL){
			verify(tt, n->scope, node);
		}
	}else if(n->type == TYPE_OPERATOR){
		n->scope = scope;
		verify(tt, n->scope, L(n));
		verify(tt, n->scope, R(n));
	}else if(n->type == TYPE_DECL){
		if (find_variable(scope, L(n)->value)){
			printf("Variable '%s' has already been declared\n", L(n)->value);
			exit(1);
		}
		Type *t = find_type(tt, R(n)->value);
		if(t == NULL){
			printf("Type '%s' has not been defined\n", R(n)->value);
			exit(1);
		}
		add_variable(scope, L(n)->value, t);

	}else if(n->type == TYPE_IDENT){
		if(!find_variable(scope, n->value)){
			printf("Variable '%s' has not been declared\n", n->value); 
			exit(1);
		}
	}else if(n->type == TYPE_CALL_PARAMS){
		Array_Iter at = make_array_iter(&n->nodes);
		Node *node = NULL;
		while((node = (Node *)next_item(&at)) != NULL){
			verify(tt, n->scope, node);
		}
	}
}


char *load_file(char *file_path){
	FILE *f = fopen(file_path, "r");

	if (!f){
		printf("Failed to open file (%s), does the file exist?\n", file_path);
		return NULL;	
	}

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *data = (char *)malloc(size+1);
	if (!data){
		printf("Failed to allocate %ld bytes in order to load %s\n", size, file_path);
		return NULL;
	}

	fread(data, 1, size, f);
	data[size] = '\0';

	return data;
}

typedef struct Parameter{
	char *name;
	int offset;
} Parameter;

typedef struct Function{
	char *name;
	Array params;

	int offset;
} Function;

int offset_for_param(Function function, char *name){
	Array_Iter at = make_array_iter(&function.params);
	Parameter *p = (Parameter *)next_item(&at);
	while(p != NULL){
		if(strcmp(p->name, name) == 0){
			return p->offset;
		}
		p = (Parameter *)next_item(&at);
	}
	return -1;
}

void print_function(Function function){
	printf("%s\n", function.name);
	Array_Iter at = make_array_iter(&function.params);

	Parameter *p = (Parameter *)next_item(&at);
	while(p!=NULL){
		printf("Name: %s, Offset: %d\n", p->name, p->offset);
		p = next_item(&at);
	}
}

//This could probably be a macro but I will wait until it is finalized, eventually I wont just be printing this stuff out and will actually be building code
void emit_instruction(Code *code, unsigned int op, int a, int b, int c){
	Inst i = make_inst(op, a, b, c);
	add_inst(code, i);
}
void emit_call(Code *code, Function f, Node *operator);
void emit_operator(Code *code, Function f, Node *operator);
void emit_operand(Code *code, Function f, Node *operand){
	//printf("Operand start\n");
	if(operand->type == TYPE_IDENT){
		int offset = offset_for_param(f, operand->value);
		emit_instruction(code, INST_LOAD_RI, 0, 9, offset);
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}else if (operand->type == TYPE_OPERATOR){
		emit_operator(code, f, operand);
	}else if( operand->type == TYPE_CALL){
		emit_call(code, f, operand);
	}else if(operand->type == TYPE_INTEGER){
		int val = atoi(operand->value);
		emit_instruction(code, INST_PUSH_I, val, -1, -1);
	}else{
		printf("TODO: only identifier operands are currently implemented\n");
	}
	//printf("Operand stop\n");
}

void emit_call(Code *code, Function f, Node *operator){
	emit_instruction(code, INST_PUSH_R, 9, -1, -1);//Save frame pointer
	emit_instruction(code, INST_MOV_R, 9, 8, -1);//Move stack pointer into frame pointer
	Node *params = CHILD(operator, 1);
	for (int i = 0;i < params->nodes.item_count; i++){
		emit_operand(code, f, CHILD(params, i));
	}
	int call_location = lookup_label(code, CHILD(operator, 0)->value);
	if( call_location == -1){
		printf("Trying to call function '%s' but it doesn't seem to exist yet\n", CHILD(operator, 0)->value);
	}
	emit_instruction(code, INST_CALL_I, call_location, -1, -1);
}

//This function is very stupid, every operand is pushed onto the stack but will
//just be popped right off again, could optimize later or just generate smarter code
//The goal currently is just to get any code running then I'll see about being smart
void emit_operator(Code *code, Function f, Node *operator){
	//printf("Operator start\n");
	Node *l = CHILD(operator, 0);
	Node *r = CHILD(operator, 1);
	emit_operand(code, f, l);
	emit_operand(code, f, r);
	emit_instruction(code, INST_POP_R, 0, -1, -1);
	emit_instruction(code, INST_POP_R, 1, -1, -1);
	if(operator->value[0] == '+'){
		emit_instruction(code, INST_ADD_R, 0, 0, 1);
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}else if(operator->value[0] == '-'){
		emit_instruction(code, INST_SUB_R, 0, 0, 1);
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}else if(operator->value[0] == '*'){
		emit_instruction(code, INST_MUL_R, 0, 0, 1);
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}else if (operator->value[0] == '/'){
		emit_instruction(code, INST_DIV_R, 0, 0, 1);
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}else{
		printf("operator '%s' hasn't yet been implemented\n", operator->value);
	}
	//printf("Operator stop\n");
}

void emit_function(Code *code, Node *func){
	//printf("Function start\n");

	Function f;
	f.offset = 0;
	//Maybe a copy would be good so that the node doesn't need to stay in memory
	f.name = CHILD(func, 0)->value;
	f.params = make_array(sizeof(Parameter));

	add_label(code, f.name, code->length);

	printf("Emitting code for function '%s'\n", f.name);

	//Deal with parameters, node code needs to be emitted as they will be pushed
	//onto the stack by the caller.
	Node *params = CHILD(func, 1);
	for (int i = 0; i<params->nodes.item_count; i++){
		Node *decl = CHILD(params, i);
		
		Parameter p;
		p.name = CHILD(decl, 0)->value;
		p.offset = f.offset++;

		add_item(&f.params, (void *)&p);
	}
	
	//Now we need to go through each expression in the block and emit the appropriate
	//code. This will become more complex when I introduce control flow
	Node* block = CHILD(func, 3);
	for( int i = 0; i<block->nodes.item_count; i++){
		Node *line = CHILD(block, i);
		if (line->type == TYPE_DECL){
			Parameter p;
			p.name = CHILD(line, 0)->value;
			p.offset = f.offset++;

			add_item(&f.params, (void *)&p);

			emit_instruction(code, INST_PUSH_I, 0, -1, -1);
		}else if(line->type == TYPE_OPERATOR){
			//Need to do a better job of checking this
			if (line->value[0] == '='){
				emit_operand(code, f, CHILD(line, 1));

				int offset = offset_for_param(f, CHILD(line, 0)->value);
				//printf("Assignment start\n");
				emit_instruction(code, INST_ADD_I, 0, 9, offset); 
				emit_instruction(code, INST_POP_R, 1, -1, -1);
				emit_instruction(code, INST_SAVE_R, 0, 1, -1);	
				//printf("Assignment stop\n");
			}
		}else if (line->type == TYPE_CALL){
			emit_operand(code, f, line);
		}else{
			printf("Not really sure what to do with node of value '%s'\n", line->value);
		}
	}
	int offset = offset_for_param(f, "return");
	if(offset != -1){
		emit_instruction(code, INST_LOAD_RI, 0, 9, offset);
	}
	//Should actually just be a sub from SP
	while(pop_item(&f.params)){
		emit_instruction(code, INST_POP, -1, -1, -1);
	}
	emit_instruction(code, INST_POP_R, 9, -1, -1);
	if(offset != -1){
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}

	if( strcmp(CHILD(func, 0)->value, "main") == 0 ){
		emit_instruction(code, INST_HALT, -1, -1, -1);
	}else{
		emit_instruction(code, INST_RET, -1, -1, -1);
	}
}

void emit_functions(Code *code, Node *functions){
	
	for (int i = 0; i< functions->nodes.item_count; i++){
		emit_function(code, CHILD(functions, i));
	}
}

int main(int argc, char **argv){

	if(argc != 2){
		printf("Please specify the file you wish to compile\n");
		return 0;
	}

	Type_Table* tt = make_type_table();
	add_built_in_types(tt);

	Parser p = make_parser(argv[1]);

	Node *global = parse_global(&p);
	print_node(&p, global);

	Scope *global_scope = create_scope(NULL);
	verify(tt, global_scope, global);

	Code code = make_code();

	emit_functions(&code, global);

	show_code(&code);

	int start = lookup_label(&code, "main");
	if(start == -1){
		printf("Your program needs a main function");
	}else{
		printf("Looks like main is at '%d'\n", start);
		Interp interp;
		interpret(&interp, &code, start);
		printf("%d\n", interp.stack[interp.reg[SP]]);
	}

	//emit_function(global->nodes.nodes);

	//print_type_table(tt);

	free(p.src);

	return 0;
}
