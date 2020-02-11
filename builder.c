#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "builder.h"
#include "dsm.h"
#include "lib.h"

typedef struct Parameter{
	char *name;
	int offset;
} Parameter;

typedef struct Function{
	char *name;
	Array params;

	int offset;

	// Cannot remember why this is
	int block_start;
	int block_end;
} Function;

void push_parameter(Function *f, char* value){
	Parameter p;
	p.name = value;
	p.offset = f->offset++;
	add_item(&f->params, (void *)&p);
}

void pop_parameters(Function *f, unsigned int count){
	f->params.item_count -= count;
	if(f->params.item_count < 0){
		printf("WARNING popped %d parameters off of function %s, byt we only had %d\n", count, f->name, f->params.item_count + count);
		f->params.item_count = 0;
	}
}

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
	i.source_line_number = code->current_source_line;
	add_inst(code, i);
}

void emit_call(Code *code, Scope *scope, Function *f, Node *operator);
void emit_operator(Code *code, Scope *scope, Function *f, Node *operator);
void emit_operand(Code *code, Scope *scope, Function *f, Node *operand){	
	update_line(code, operand->source_location.line);
	if(operand->type == TYPE_IDENT){
		int offset = offset_for_param(f, operand->value);
		if(offset == -1){
			printf("In emit_operand, paramerter '%s' not found\n", operand->value);
			exit(0);
		}
		emit_instruction(code, INST_LOAD_RI, R0, FP, offset);
		emit_instruction(code, INST_PUSH_R, R0, -1, -1);
	}else if (operand->type == TYPE_OPERATOR){
		emit_operator(code, scope, f, operand);
	}else if( operand->type == TYPE_CALL){
		emit_call(code, scope, f, operand);
	}else if(operand->type == TYPE_INTEGER){
		int val = atoi(operand->value);
		emit_instruction(code, INST_PUSH_I, val, -1, -1);
	}else{
		printf("%d is not a type of operand we handle\n", operand->type);
	}
}

void emit_call(Code *code, Scope *scope, Function *f, Node *operator){
	update_line(code, operator->source_location.line);
	Identifier* r = lookup_identifier(scope, CHILD(operator, 0)->value);
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
		emit_operand(code, scope, f, CHILD(params, i));	// Put call parameters on stack
	}

	if(!is_external_call){
		int call_location = index_of_name(scope, CHILD(operator, 0)->value);
		if( call_location == -1){
			printf("Trying to call function '%s' but it doesn't seem to exist yet\n", CHILD(operator, 0)->value);
		}	
		emit_instruction(code, INST_MOV_R, FP, R2, -1);
		emit_instruction(code, INST_CALL_I, call_location, -1, -1);
		emit_instruction(code, INST_SWAP_STACK, -1, -1, -1);
		emit_instruction(code, INST_POP_R, RET, -1, -1);
	}else{
		int external_index = lookup_external_index(CHILD(operator, 0)->value);
		if(external_index == -1){
			printf("External function '%s' is not defined\n", CHILD(operator, 0)->value);
			exit(0);
		}
		emit_instruction(code, INST_EXT_CALL_I, external_index, -1, -1);
	}
}

//This function is very stupid, every operand is pushed onto the stack but will
//just be popped right off again, could optimize later or just generate smarter code
//The goal currently is just to get any code running then I'll see about being smart
void emit_operator(Code *code, Scope *scope, Function *f, Node *operator){
	update_line(code, operator->source_location.line);

	Node *l = CHILD(operator, 0);
	Node *r = CHILD(operator, 1);
	emit_operand(code, scope,  f, l);
	emit_operand(code, scope,  f, r);
	emit_instruction(code, INST_POP_R, R1, -1, -1);
	emit_instruction(code, INST_POP_R, R0, -1, -1);
	
	if(strcmp(operator->value, "==") == 0){
		// This is ugly and can likely be nicer
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
		printf("Emiting halt instruction\n");
		emit_instruction(code, INST_HALT, -1, -1, -1);
	}else{
		emit_instruction(code, INST_RET, -1, -1, -1);
	}
}

void emit_expression(Code *code, Scope *scope, Function *f, Node *child){
	update_line(code, child->source_location.line);
	if(child->type == TYPE_OPERATOR){
		emit_operator(code, scope, f, child);
	}else{
		emit_operand(code, scope, f, child);
	}
}

void emit_block(Code *code, Scope *scope, Function *f, Node *block);

void emit_if_block(Code *code, Scope *scope, Function *f, Node *block){
	update_line(code, block->source_location.line);
	

	Node *expression = CHILD(block, 0);
	emit_expression(code, scope, f, expression);
	emit_instruction(code, INST_POP_R, R0, -1, -1);
	emit_instruction(code, INST_CMP_I, R0, 1, -1);
	
	unsigned int start = code->length;
	emit_instruction(code, INST_JUMP_NEQL, -1, -1, -1);	// Emit place holder location
	Node *b = CHILD(block, 1);
	emit_block(code, scope, f, b);
	emit_instruction(code, INST_GOTO_I, -1, -1, -1); // Place holder for end of conditional
	code->code[start].a = code->length; //Patch location
}

void emit_conditional(Code *code, Scope *scope, Function *f, Node *conditional){
	unsigned int start = code->length;
	unsigned int end = 0;
	update_line(code, conditional->source_location.line);

	Array_Iter at = make_array_iter(&conditional->nodes);
	Node *child = NULL;
	while((child = (Node *)next_item(&at)) != NULL){
		if(child->type == TYPE_IF || child->type == TYPE_ELSE_IF){
			emit_if_block(code, scope, f, child);
		}else if (child->type == TYPE_ELSE){
			emit_block(code, scope, f, CHILD(child, 0));
			end = code->length;
			break;
		}
		end = code->length;
	}
	// Bit hacky but we are just going to find those bad gotos and making them jump to end
	for(int i = start; i < end; i++){
		if(code->code[i].inst == INST_GOTO_I && code->code[i].a == -1){
			code->code[i].a = end;
		}
	}
}

void emit_block(Code *code, Scope *scope, Function *f,  Node *block){
	update_line(code, block->source_location.line);
	
	f->block_start = code->length;
	unsigned int params_pushed = 0;

	// Refactor: Should I use an iter?
	for (int i = 0; i<block->nodes.item_count; i++){
		Node *line = CHILD(block, i);
		if (line->type == TYPE_DECL){
			push_parameter(f, CHILD(line, 0)->value);
			params_pushed++;
			
			emit_instruction(code, INST_PUSH_I, 0, -1, -1);
		}else if(line->type == TYPE_OPERATOR){
			if (strcmp(line->value, "=") == 0){
				emit_operand(code, scope, f, CHILD(line, 1));
				char *val = CHILD(line, 0)->value;
				int offset = offset_for_param(f, val);
				if(offset == -1){
					printf("Cannot output assignment because %s has not been defined\n", val);
					exit(0);
				}	
				emit_instruction(code, INST_ADD_I, 0, 9, offset); 
				emit_instruction(code, INST_POP_R, 1, -1, -1);
				emit_instruction(code, INST_SAVE_R, 0, 1, -1);	
			}else{
				//@TODO then what!?
			}
		}else if (line->type == TYPE_CALL){
			emit_operand(code, scope, f, line);
			emit_instruction(code, INST_POP, -1, -1, -1); // Unused return value
		}else if (line->type == TYPE_CONDITIONAL){
			emit_conditional(code, scope, f, line);
		}else if (line->type == TYPE_RETURN){
			Node *child = CHILD(line, 0);
			emit_expression(code, scope, f, child);
			emit_return(code, f);
		}else{
			printf("Emitting block, '%s' of type %d is unkown\n", line->value, line->type);
		}
	}
	pop_parameters(f, params_pushed);
	f->block_end = code->length;
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

void emit_function(Code *code, Scope *scope, Node *func){
	update_line(code, func->source_location.line);

	Function f;
	f.offset = 0;
	
	f.name = CHILD(func, 0)->value; // Refactor: Should this be a copy instead?
	f.params = make_array(sizeof(Parameter));

	// If the function is external we don't want to patch the location
	// as they have fixed 'locations' which are just array indices

	//TODO what if n is null?
	Identifier *n = lookup_identifier(scope, f.name);
	if(!n->is_external){
		set_location(scope, f.name, code->length);
	}

	// Reserve space on stack for function parameters
	Node *params = CHILD(func, 1);
	for (int i = 0; i < params->nodes.item_count; i++){
		Node *decl = CHILD(params, i);
	
		push_parameter(&f, CHILD(decl, 0)->value);
	}

	Node* block = CHILD(func, 3);
	emit_block(code, scope, &f, block);	// Emit the body of the function
}

void build_code(Node *node, Code *code, Scope *scope){
	Array_Iter nodes = make_array_iter(&node->nodes);
	Node *n = NULL;
	while((n = (Node *)next_item(&nodes)) != NULL){
		if(n->type == TYPE_FUNCTION){
			emit_function(code, scope, n);
		}else{
			printf("Only functions can currently be defined at the top level\n");
		}
	}
}
