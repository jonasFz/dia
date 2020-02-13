#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "builder.h"
#include "dsm.h"
#include "lib.h"

typedef struct Parameter{
	char *name;
	int offset;
	int bytes;
} Parameter;

typedef struct Function{
	char *name;
	Array params;

	int offset;

	// Cannot remember why this is
	int block_start;
	int block_end;
} Function;

typedef struct{
	Code *code;
	Scope *scope;
	Function *func;
}Build_Context;

void push_parameter(Function *f, char* value, int bytes){
	Parameter p;
	p.name = value;
	p.offset = f->offset;
	p.bytes = bytes;
	f->offset += bytes;
	add_item(&f->params, (void *)&p);
}

void pop_parameters(Function *f, unsigned int count){
	int param_count = f->params.item_count;
	for(int i = 0; i < count; i++){
		f->offset -= ((Parameter *)get_item(&f->params, param_count - i))->bytes;
	}
	f->params.item_count -= count;
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

void emit_instruction(Build_Context *bc, unsigned int op, int a, int b, int c){
	Inst i = make_inst(op, a, b, c);
	i.source_line_number = bc->code->current_source_line;
	add_inst(bc->code, i);
}

void emit_call(Build_Context *bc, Node *operator);
void emit_operator(Build_Context *bc, Node *operator);
void emit_operand(Build_Context *bc, Node *operand){	
	update_line(bc->code, operand->source_location.line);
	if(operand->type == TYPE_IDENT){
		int offset = offset_for_param(bc->func, operand->value);
		if(offset == -1){
			printf("In emit_operand, paramerter '%s' not found\n", operand->value);
			exit(0);
		}
		emit_instruction(bc, INST_LOAD_RI, R0, FP, offset);
		emit_instruction(bc, INST_PUSH_R, R0, -1, -1);
	}else if (operand->type == TYPE_OPERATOR){
		emit_operator(bc, operand);
	}else if( operand->type == TYPE_CALL){
		emit_call(bc, operand);
	}else if(operand->type == TYPE_INTEGER){
		int val = atoi(operand->value);
		emit_instruction(bc, INST_PUSH_I, val, -1, -1);
	}else{
		printf("%d is not a type of operand we handle\n", operand->type);
	}
}

void emit_call(Build_Context *bc, Node *call){
	update_line(bc->code, call->source_location.line);
	Identifier* r = lookup_identifier(bc->scope, CALL_VALUE(call)->value);
	if(r == NULL){
		printf("Attempt to call function '%s' failed because it could not be located. Does it exist?\n", CALL_VALUE(call)->value);
		exit(1);
	}
	int is_external_call = r->is_external;
	
	if(!is_external_call){
		emit_instruction(bc, INST_PUSH_R, RET, -1, -1); 
		emit_instruction(bc, INST_PUSH_R, FP, -1, -1);
		emit_instruction(bc, INST_MOV_R, R2, SP, -1);	// Temporarily save SP
	}
	
	Node *params = CALL_PARAMETERS(call);
	for (int i = 0;i < params->nodes.item_count; i++){
		emit_operand(bc, CHILD(params, i));	// Put call parameters on stack
	}

	if(!is_external_call){
		int call_location = index_of_name(bc->scope, CALL_VALUE(call)->value);
		if( call_location == -1){
			printf("Trying to call function '%s' but it doesn't seem to exist yet\n", CALL_VALUE(call)->value);
		}	
		emit_instruction(bc, INST_MOV_R, FP, R2, -1);
		emit_instruction(bc, INST_CALL_I, call_location, -1, -1);
		emit_instruction(bc, INST_SWAP_STACK, -1, -1, -1);
		emit_instruction(bc, INST_POP_R, RET, -1, -1);
	}else{
		int external_index = lookup_external_index(CALL_VALUE(call)->value);
		if(external_index == -1){
			printf("External function '%s' is not defined\n", CALL_VALUE(call)->value);
			exit(0);
		}
		emit_instruction(bc, INST_EXT_CALL_I, external_index, -1, -1);
	}
}

//This function is very stupid, every operand is pushed onto the stack but will
//just be popped right off again, could optimize later or just generate smarter code
//The goal currently is just to get any code running then I'll see about being smart
void emit_operator(Build_Context *bc, Node *operator){
	update_line(bc->code, operator->source_location.line);

	Node *l = CHILD(operator, 0);
	Node *r = CHILD(operator, 1);
	emit_operand(bc, l);
	emit_operand(bc, r);
	emit_instruction(bc, INST_POP_R, R1, -1, -1);
	emit_instruction(bc, INST_POP_R, R0, -1, -1);
	
	if(strcmp(operator->value, "==") == 0){
		// This is ugly and can likely be nicer
		emit_instruction(bc, INST_SUB_R, R0, R0, R1);
		emit_instruction(bc, INST_CMP_I, R0, 0, -1);
		emit_instruction(bc, INST_JUMP_NEQL, bc->code->length+3, -1, -1);
		
		emit_instruction(bc, INST_PUSH_I, 1, -1, -1);
		emit_instruction(bc, INST_GOTO_I, bc->code->length+2, -1, -1); 
		emit_instruction(bc, INST_PUSH_I, 0, -1, -1);

	}else if(operator->value[0] == '+'){
		emit_instruction(bc, INST_ADD_R, R0, R0, R1);
		emit_instruction(bc, INST_PUSH_R, R0, -1, -1);
	}else if(operator->value[0] == '-'){
		emit_instruction(bc, INST_SUB_R, R0, R0, R1);
		emit_instruction(bc, INST_PUSH_R, 0, -1, -1);
	}else if(operator->value[0] == '*'){
		emit_instruction(bc, INST_MUL_R, R0, R0, R1);
		emit_instruction(bc, INST_PUSH_R, 0, -1, -1);
	}else if (operator->value[0] == '/'){
		emit_instruction(bc, INST_DIV_R, R0, R0, R1);
		emit_instruction(bc, INST_PUSH_R, 0, -1, -1);
	}else{
		printf("operator '%s' hasn't yet been implemented\n", operator->value);
	}
}

void emit_return(Build_Context *bc){
	// The block will leave a value on the stack, here we pop it off
	// so we can put it at the bottom of the stack later as the return value
	// TYPE will need to figure out how to return big things.
	emit_instruction(bc, INST_POP_R, 0, -1, -1);
	
	//Clean up the stack before returing
	int param_count = bc->func->params.item_count;	
	emit_instruction(bc, INST_SUB_I, SP, SP, param_count*4);

	//Restore the old frame pointer for the calling function
	if( strcmp(bc->func->name, "main") != 0){
		emit_instruction(bc, INST_POP_R, FP, -1, -1);
		emit_instruction(bc, INST_PUSH_R, R0, -1, -1);
	}
	if(strcmp(bc->func->name, "main") == 0){
		printf("Emiting halt instruction\n");
		emit_instruction(bc, INST_HALT, -1, -1, -1);
	}else{
		emit_instruction(bc, INST_RET, -1, -1, -1);
	}
}

void emit_expression(Build_Context *bc, Node *child){
	update_line(bc->code, child->source_location.line);
	if(child->type == TYPE_OPERATOR){
		emit_operator(bc, child);
	}else{
		emit_operand(bc, child);
	}
}

void emit_block(Build_Context *bc, Node *block);

void emit_if_block(Build_Context *bc, Node *block){
	update_line(bc->code, block->source_location.line);
	

	Node *expression = IF_EXPRESSION(block);
	emit_expression(bc, expression);
	emit_instruction(bc, INST_POP_R, R0, -1, -1);
	emit_instruction(bc, INST_CMP_I, R0, 1, -1);
	
	unsigned int start = bc->code->length;
	emit_instruction(bc, INST_JUMP_NEQL, -1, -1, -1);	// Emit place holder location
	Node *b = IF_BLOCK(block);
	emit_block(bc, b);
	emit_instruction(bc, INST_GOTO_I, -1, -1, -1); // Place holder for end of conditional
	bc->code->code[start].a = bc->code->length; //Patch location
}

void emit_conditional(Build_Context *bc, Node *conditional){
	unsigned int start = bc->code->length;
	unsigned int end = 0;
	update_line(bc->code, conditional->source_location.line);

	Array_Iter at = make_array_iter(&conditional->nodes);
	Node *child = NULL;
	while((child = (Node *)next_item(&at)) != NULL){
		if(child->type == TYPE_IF || child->type == TYPE_ELSE_IF){
			emit_if_block(bc, child);
		}else if (child->type == TYPE_ELSE){
			emit_block(bc, CONDITIONAL_ELSE(child));
			end = bc->code->length;
			break;
		}
		end = bc->code->length;
	}
	// Bit hacky but we are just going to find those bad gotos and making them jump to end
	for(int i = start; i < end; i++){
		if(bc->code->code[i].inst == INST_GOTO_I && bc->code->code[i].a == -1){
			bc->code->code[i].a = end;
		}
	}
}

void emit_block(Build_Context *bc, Node *block){
	update_line(bc->code, block->source_location.line);
	
	bc->func->block_start = bc->code->length;
	unsigned int params_pushed = 0;

	// Refactor: Should I use an iter?
	for (int i = 0; i<block->nodes.item_count; i++){
		Node *line = CHILD(block, i);
		if (line->type == TYPE_DECL){
			push_parameter(bc->func, DECL_VALUE(line)->value, 4);
			params_pushed++;
			
			emit_instruction(bc, INST_PUSH_I, 0, -1, -1);
		}else if(line->type == TYPE_OPERATOR){
			if (strcmp(line->value, "=") == 0){
				emit_operand(bc, OPERATOR_RIGHT(line));
				char *val = OPERATOR_LEFT(line)->value;
				int offset = offset_for_param(bc->func, val);
				if(offset == -1){
					printf("Cannot output assignment because %s has not been defined\n", val);
					exit(0);
				}	
				emit_instruction(bc, INST_ADD_I, 0, 9, offset); 
				emit_instruction(bc, INST_POP_R, 1, -1, -1);
				emit_instruction(bc, INST_SAVE_R, 0, 1, -1);	
			}else{
				//@TODO then what!?
			}
		}else if (line->type == TYPE_CALL){
			emit_operand(bc, line);
			emit_instruction(bc, INST_POP, -1, -1, -1); // Unused return value
		}else if (line->type == TYPE_CONDITIONAL){
			emit_conditional(bc, line);
		}else if (line->type == TYPE_RETURN){
			Node *child = RETURN_EXPRESSION(line);
			emit_expression(bc, child);
			emit_return(bc);
		}else{
			printf("Emitting block, '%s' of type %d is unkown\n", line->value, line->type);
		}
	}
	pop_parameters(bc->func, params_pushed);
	bc->func->block_end = bc->code->length;
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
	
	f.name = FUNCTION_NAME(func)->value; // Refactor: Should this be a copy instead?
	f.params = make_array(sizeof(Parameter));

	//TODO what if n is null?
	Identifier *n = lookup_identifier(scope, f.name);
	if(!n->is_external){
		set_location(scope, f.name, code->length);
	}

	// Reserve space on stack for function parameters
	Node *params = FUNCTION_PARAMETERS(func);
	for (int i = 0; i < params->nodes.item_count; i++){
		Node *decl = CHILD(params, i);
	
		push_parameter(&f, DECL_VALUE(decl)->value, 4);
	}

	Node* block = FUNCTION_BLOCK(func);
	Build_Context bc;
	bc.code = code;
	bc.scope = scope;
	bc.func = &f;
	emit_block(&bc, block);	// Emit the body of the function
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
