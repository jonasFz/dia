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

	int block_start;
	int block_end;
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
	if(operand->type == TYPE_IDENT){
		int offset = offset_for_param(f, operand->value);
		emit_instruction(code, INST_LOAD_RI, R0, FP, offset);
		emit_instruction(code, INST_PUSH_R, R0, -1, -1);
	}else if (operand->type == TYPE_OPERATOR){
		emit_operator(code, nt, f, operand);
	}else if( operand->type == TYPE_CALL){
		emit_call(code, nt, f, operand);
	}else if(operand->type == TYPE_INTEGER){
		int val = atoi(operand->value);
		emit_instruction(code, INST_PUSH_I, val, -1, -1);
	}else{
		printf("%d is not a type of operand we handle\n", operand->type);
	}
}

void emit_call(Code *code, Name_Table *nt, Function *f, Node *operator){
	Row* r = lookup_name(nt, CHILD(operator, 0)->value);
	if(r == NULL){
		printf("Attempt to call function '%s' failed because it could not be located. Does it exist?\n", CHILD(operator, 0)->value);
		exit(1);
	}
	int is_external_call = r->is_external;
	
	if(!is_external_call){
		emit_instruction(code, INST_PUSH_R, RET, -1, -1); 
		emit_instruction(code, INST_PUSH_R, FP, -1, -1);
		emit_instruction(code, INST_MOV_R, R2, SP, -1);	// Temporarily save SP
	}
	
	Node *params = CHILD(operator, 1);
	for (int i = 0;i < params->nodes.item_count; i++){
		emit_operand(code, nt, f, CHILD(params, i));	// Put call parameters on stack
	}

	if(!is_external_call){
		int call_location = index_of_name(nt, CHILD(operator, 0)->value);
		if( call_location == -1){
			printf("Trying to call function '%s' but it doesn't seem to exist yet\n", CHILD(operator, 0)->value);
		}	
		emit_instruction(code, INST_MOV_R, FP, R2, -1);
		emit_instruction(code, INST_CALL_I, call_location, -1, -1);
		emit_instruction(code, INST_SWAP_STACK, -1, -1, -1);
		emit_instruction(code, INST_POP_R, RET, -1, -1);
	}else{	
		emit_instruction(code, INST_EXT_CALL_I, index_of_name(nt, CHILD(operator, 0)->value), -1, -1);
	}
}

//This function is very stupid, every operand is pushed onto the stack but will
//just be popped right off again, could optimize later or just generate smarter code
//The goal currently is just to get any code running then I'll see about being smart
void emit_operator(Code *code, Name_Table *nt, Function *f, Node *operator){
	Node *l = CHILD(operator, 0);
	Node *r = CHILD(operator, 1);
	emit_operand(code, nt,  f, l);
	emit_operand(code, nt,  f, r);
	emit_instruction(code, INST_POP_R, R1, -1, -1);
	emit_instruction(code, INST_POP_R, R0, -1, -1);
	
	if(strcmp(operator->value, "==") == 0){
		emit_instruction(code, INST_SUB_R, R0, R0, R1);
		emit_instruction(code, INST_CMP_I, R0, 0, -1);
		emit_instruction(code, INST_JUMP_NEQL, code->length+3, -1, -1);
		
		emit_instruction(code, INST_PUSH_I, 1, -1, -1);
		emit_instruction(code, INST_GOTO_I, code->length+2, -1, -1); 
		emit_instruction(code, INST_PUSH_I, 0, -1, -1);

	}else if(operator->value[0] == '+'){
		emit_instruction(code, INST_ADD_R, R0, R0, R1);
		emit_instruction(code, INST_PUSH_R, R0, -1, -1);
	}else if(operator->value[0] == '-'){
		emit_instruction(code, INST_SUB_R, R0, R0, R1);
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}else if(operator->value[0] == '*'){
		emit_instruction(code, INST_MUL_R, R0, R0, R1);
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}else if (operator->value[0] == '/'){
		emit_instruction(code, INST_DIV_R, R0, R0, R1);
		emit_instruction(code, INST_PUSH_R, 0, -1, -1);
	}else{
		printf("operator '%s' hasn't yet been implemented\n", operator->value);
	}
}

void emit_return(Code *code, Function *f){
	
	// The block will leave a value on the stack, here we pop it off
	// so we can put it at the bottom of the stack later as the return value
	// TYPE will need to figure out how to return big things.
	emit_instruction(code, INST_POP_R, 0, -1, -1);
	
	//Clean up the stack before returing
	int param_count = f->params.item_count;	
	emit_instruction(code, INST_SUB_I, SP, SP, param_count);

	//Restore the old frame pointer for the calling function
	if( strcmp(f->name, "main") != 0){
		emit_instruction(code, INST_POP_R, FP, -1, -1);
		emit_instruction(code, INST_PUSH_R, R0, -1, -1);
	}
	if(strcmp(f->name, "main") == 0){
		emit_instruction(code, INST_HALT, -1, -1, -1);
	}else{
		emit_instruction(code, INST_RET, -1, -1, -1);
	}
}

void emit_expression(Code *code, Name_Table *nt, Function *f, Node *child){
	if(child->type == TYPE_OPERATOR){
		emit_operator(code, nt, f, child);
	}else{
		emit_operand(code, nt, f, child);
	}
}

void emit_block(Code *code, Name_Table *nt, Function *f, Node *block);

void emit_if_block(Code *code, Name_Table *nt, Function *f, Node *block){
	Node *expression = CHILD(block, 0);
	emit_expression(code, nt, f, expression);
	emit_instruction(code, INST_POP_R, R0, -1, -1);
	emit_instruction(code, INST_CMP_I, R0, 1, -1);
	
	emit_instruction(code, INST_JUMP_NEQL, -1, -1, -1);	// Emit place holder location
	Node *b = CHILD(block, 1);
	emit_block(code, nt, f, b);
	code->code[f->block_start-1].a = f->block_end;		// Patch location
}

void emit_block(Code *code, Name_Table *nt, Function *f,  Node *block){
	f->block_start = code->length;
	
	// Refactor: Should I use an iter?
	for (int i = 0; i<block->nodes.item_count; i++){
		Node *line = CHILD(block, i);
		if (line->type == TYPE_DECL){
			Parameter p;
			p.name = CHILD(line, 0)->value;
			p.offset = f->offset++;
			add_item(&f->params, (void *)&p);

			emit_instruction(code, INST_PUSH_I, 0, -1, -1);
		}else if(line->type == TYPE_OPERATOR){
			if (strcmp(line->value, "=") == 0){
				emit_operand(code, nt, f, CHILD(line, 1));
				int offset = offset_for_param(f, CHILD(line, 0)->value);

				emit_instruction(code, INST_ADD_I, 0, 9, offset); 
				emit_instruction(code, INST_POP_R, 1, -1, -1);
				emit_instruction(code, INST_SAVE_R, 0, 1, -1);	
			}
		}else if (line->type == TYPE_CALL){
			emit_operand(code, nt, f, line);
			emit_instruction(code, INST_POP, -1, -1, -1); // Unused return value
		}else if (line->type == TYPE_IF){
			emit_if_block(code, nt, f, line);
		}else if (line->type == TYPE_RETURN){
			Node *child = CHILD(line, 0);
			emit_expression(code, nt, f, child);
			emit_return(code, f);
		}else{
			printf("Not really sure what to do with node of value '%s'\n", line->value);
		}
	}
	f->block_end = code->length;
	printf("Block from %d to %d\n", f->block_start, f->block_end);
}

void show_function(Function *f){
	printf("Function: %s[\n", f->name);

	Array_Iter at = make_array_iter(&f->params);
	Parameter *p = NULL;

	while((p = (Parameter *)next_item(&at)) != NULL){
		printf("	%s %d\n", p->name, p->offset);
	}
	printf("]\n");
}

void emit_function(Code *code, Name_Table *nt, Node *func){
	Function f;
	f.offset = 0;
	
	f.name = CHILD(func, 0)->value; // Refactor: Should this be a copy instead?
	f.params = make_array(sizeof(Parameter));

	// If the function is external we don't want to patch the location
	// as they have fixed 'locations' which are just array indices
	if(!lookup_name(nt, f.name)->is_external){
		set_location(nt, f.name, code->length);
	}

	// Reserve space on stack for function parameters
	Node *params = CHILD(func, 1);
	for (int i = 0; i < params->nodes.item_count; i++){
		Node *decl = CHILD(params, i);
		
		Parameter p;
		p.name = CHILD(decl, 0)->value;
		p.offset = f.offset++;

		add_item(&f.params, (void *)&p);
	}

	Node* block = CHILD(func, 3);
	emit_block(code, nt, &f, block);	// Emit the body of the function

	show_function(&f);
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
			printf("Only functions can currently be defined at the top level\n");
		}
	}
}
