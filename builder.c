#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "builder.h"
#include "dsm.h"

typedef struct Parameter{
	char *name;
	int offset;
} Parameter;

typedef struct Function{
	char *name;
	Array params;

	int offset;
} Function;

int offset_for_param(Function *function, char *name){
	Array_Iter at = make_array_iter(&function->params);
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

void emit_call(Code *code, Name_Table *nt, Function *f, Node *operator);
void emit_operator(Code *code,Name_Table *nt, Function *f, Node *operator);
void emit_operand(Code *code, Name_Table *nt, Function *f, Node *operand){
	//printf("Operand start\n");
	if(operand->type == TYPE_IDENT){
		int offset = offset_for_param(f, operand->value);
		emit_instruction(code, INST_LOAD_RI, 0, 9, offset);
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}else if (operand->type == TYPE_OPERATOR){
		emit_operator(code, nt, f, operand);
	}else if( operand->type == TYPE_CALL){
		emit_call(code, nt, f, operand);
	}else if(operand->type == TYPE_INTEGER){
		int val = atoi(operand->value);
		emit_instruction(code, INST_PUSH_I, val, -1, -1);
	}else{
		printf("TODO: only identifier operands are currently implemented\n");
	}
	//printf("Operand stop\n");
}

void emit_call(Code *code, Name_Table *nt, Function *f, Node *operator){
	emit_instruction(code, INST_PUSH_R, 9, -1, -1);//Save frame pointer
	emit_instruction(code, INST_MOV_R, 9, 8, -1);//Move stack pointer into frame pointer
	Node *params = CHILD(operator, 1);
	for (int i = 0;i < params->nodes.item_count; i++){
		emit_operand(code, nt, f, CHILD(params, i));
	}
	int call_location = index_of_name(nt, CHILD(operator, 0)->value);
	if( call_location == -1){
		printf("Trying to call function '%s' but it doesn't seem to exist yet\n", CHILD(operator, 0)->value);
	}
	emit_instruction(code, INST_CALL_I, call_location, -1, -1);
}

//This function is very stupid, every operand is pushed onto the stack but will
//just be popped right off again, could optimize later or just generate smarter code
//The goal currently is just to get any code running then I'll see about being smart
void emit_operator(Code *code, Name_Table *nt, Function *f, Node *operator){
	//printf("Operator start\n");
	Node *l = CHILD(operator, 0);
	Node *r = CHILD(operator, 1);
	emit_operand(code, nt,  f, l);
	emit_operand(code, nt,  f, r);
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

void emit_return(Code *code, Function *f){
	// The block will leave a value on the stack, here we pop it off
	// so we can put it at the bottom of the stack later as the return value
	// TYPE will need to figure out how to return big things.
	emit_instruction(code, INST_POP_R, 0, -1, -1);

	//Clean up the stack before returing
	int param_count = f->params.item_count;
	//Maybe we should make stack math seperate?
	emit_instruction(code, INST_SUB_I, 8, 8, param_count);

	emit_instruction(code, INST_POP_R, 9, -1, -1);

	//The return value we saved earlier now pushed back on
	emit_instruction(code, INST_PUSH_R, 0, -1, -1);

	//Kind of hacky and probably slow
	//Will probably make the code entry point transparent and then 
	//make main just a regular function.
	//Put a generic bit of code at the beginning of every program that
	//just calls main.
	if( strcmp(f->name, "main") == 0 ){
		emit_instruction(code, INST_HALT, -1, -1, -1);
	}else{
		emit_instruction(code, INST_RET, -1, -1, -1);
	}
}

void emit_block(Code *code, Name_Table *nt, Function *f,  Node *block){
	for (int i = 0; i<block->nodes.item_count; i++){
		Node *line = CHILD(block, i);
		if (line->type == TYPE_DECL){
			Parameter p;
			p.name = CHILD(line, 0)->value;
			p.offset = f->offset++;

			add_item(&f->params, (void *)&p);

			emit_instruction(code, INST_PUSH_I, 0, -1, -1);
		}else if(line->type == TYPE_OPERATOR){
			//Need to do a better job of checking this
			if (line->value[0] == '='){
				emit_operand(code, nt, f, CHILD(line, 1));

				int offset = offset_for_param(f, CHILD(line, 0)->value);
				//printf("Assignment start\n");
				emit_instruction(code, INST_ADD_I, 0, 9, offset); 
				emit_instruction(code, INST_POP_R, 1, -1, -1);
				emit_instruction(code, INST_SAVE_R, 0, 1, -1);	
				//printf("Assignment stop\n");
			}
		}else if (line->type == TYPE_CALL){
			emit_operand(code, nt, f, line);
		}else if (line->type == TYPE_IF){
			printf("If statement\n");
		}else if (line->type == TYPE_RETURN){
			Node *child = CHILD(line, 0);
			if(child->type == TYPE_OPERATOR){
				emit_operator(code, nt, f, child);
			}else{
				emit_operand(code, nt, f, child);
			}
			emit_return(code, f);
		}else{
			printf("Not really sure what to do with node of value '%s'\n", line->value);
		}
	}
}

void emit_function(Code *code, Name_Table *nt, Node *func){
	//printf("Function start\n");

	Function f;
	f.offset = 0;
	//Maybe a copy would be good so that the node doesn't need to stay in memory
	f.name = CHILD(func, 0)->value;
	f.params = make_array(sizeof(Parameter));

	//If the function is external we don't want to patch the location
	//as they have fixed 'locations' which are just array indices
	if(!lookup_name(nt, f.name)->is_external){
		set_location(nt, f.name, code->length);
	}

	//add_label(code, f.name, code->length);

	printf("Emitting code for function '%s'\n", f.name);

	//Deal with parameters, node code needs to be emitted as they will be pushed
	//onto the stack by the caller.
	Node *params = CHILD(func, 1);
	for (int i = 0; i < params->nodes.item_count; i++){
		Node *decl = CHILD(params, i);
		
		Parameter p;
		p.name = CHILD(decl, 0)->value;
		p.offset = f.offset++;

		add_item(&f.params, (void *)&p);
	}
	//Now we need to go through each expression in the block and emit the appropriate
	//code. This will become more complex when I introduce control flow
	Node* block = CHILD(func, 3);
	emit_block(code, nt, &f, block);
	//The block is expected to generate the return for a function
	//Will later have a stage that validates the structure and inserts
	//a return if needed.
}


Name_Table make_name_table(){
	Name_Table nt;
	nt.names = make_array(sizeof(Row));

	return nt;
}

void register_name(Name_Table *nt, char * name, int is_external){
	int length = strlen(name);
	char *cpy = (char *)malloc(sizeof(char) * length+1);
	strncpy(cpy, name, length);
	cpy[length + 1] = '\0';

	Row row;
	row.name = cpy;
	row.location = -1;
	row.is_external = is_external;

	add_item(&nt->names, (void *)&row);
}

Row* lookup_name(Name_Table *nt, char *name){
	Array_Iter at = make_array_iter(&nt->names);
	Row *current = NULL;

	int index = 0;
	while((current = (Row *)next_item(&at)) != NULL){
		if(strcmp(current->name, name) == 0){
			return current;
		}
		index++;
	}
	return NULL;
}

int index_of_name(Name_Table *nt, char *name){
	Array_Iter at = make_array_iter(&nt->names);
	Row *current = NULL;

	int index = 0;
	while((current = (Row *)next_item(&at)) != NULL){
		if(strcmp(current->name, name) == 0){
			return index;
		}
		index++;
	}
	return index;
}

void set_location(Name_Table *nt, char *name, int location){
	Row *r = NULL;
	if((r = lookup_name(nt, name)) == NULL){
		printf("'%s' has not been defined in Name_Table, cannot set location %d\n", name, location);
		return;
	}
	r->location = location;
}

void print_name_table(Name_Table* nt){
	Array_Iter at = make_array_iter(&nt->names);
	Row *row = NULL;
	int index = 0;
	while((row = (Row *)next_item(&at)) != NULL){
		printf("%2d: %s\n", index, row->name);
		index++;
	}
}

int get_location_of_name(Name_Table *nt, char *name){
	Row *r = lookup_name(nt, name);
	if(r == NULL) return -1;
	return r->location;
}

void build_code(Node *node, Code *code, Name_Table *nt){
	if(node->type != TYPE_GLOBAL){
		printf("Can only build code from a node of type GLOBAL\n");
		return;
	}
	*nt = make_name_table();
	Array_Iter nodes = make_array_iter(&node->nodes);
	Node *n = NULL;
	while((n = (Node *)next_item(&nodes)) != NULL){
		if(n->type == TYPE_FUNCTION){
			//Figure out where we want to check if the function already defined.
			int is_external = n->flags & FLAG_EXTERNAL;
			register_name(nt, CHILD(n, 0)->value, is_external);
		}else{
			printf("Only handle functions at the top level currently\n");
		}
	}
	print_name_table(nt);

	nodes = make_array_iter(&node->nodes);
	n = NULL;
	while((n = (Node *)next_item(&nodes)) != NULL){
		if(n->type == TYPE_FUNCTION){
			emit_function(code, nt, n);
		}else{
			printf("Only handle functions at the top level currently\n");
		}
	}
}
