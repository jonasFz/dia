#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "parse.h"

Type_Table* make_type_table(){
	Type_Table* tt = (Type_Table *)malloc(sizeof(Type_Table));
	tt->types = (Type *)malloc(sizeof(Type) * 256);
	tt->cap = 256;

	return tt;
}


// Speed: This should probably do a hash lookup at some point.
// As programs get big and we have alot of types this linear
// search will get too expensive. But I think Knuth said that
// premature optimization is the root of all evil - Jonas Ferencz November 5, 2019
Type* find_type(Type_Table *tt, const char *name){
	for (int i = 0; i < tt->count; i++){
		if (strcmp(name, tt->types[i].name) == 0){
			return &tt->types[i];
		}
	}
	return NULL;
}

void register_type(Type_Table *tt, const char *name, unsigned int length){
	if(tt->count == tt->cap){
		tt->types = (Type *)realloc(tt->types, tt->cap*2);
		tt->cap *= 2;
	}
	Type *t = &tt->types[tt->count++];

	long int name_length = strlen(name);

	t->name = (char *)malloc(sizeof(char) * name_length + 1);
	strncpy(t->name, name, strlen(name));
	t->name[name_length] = '\0';
	t->length = length;
}

void add_built_in_types(Type_Table *tt){
	register_type(tt, "void", 1);
	register_type(tt, "u8", 1);
	register_type(tt, "s8", 1);
	register_type(tt, "u16", 2);
	register_type(tt, "s16", 2);
	register_type(tt, "u32", 4);
	register_type(tt, "s32", 4);
	register_type(tt, "u64", 8);
	register_type(tt, "s64", 8);
	register_type(tt, "f32", 4);
	register_type(tt, "f64", 8);
}
void print_type_table(Type_Table *tt){
	for (int i = 0;i < tt->count; i++){
		printf("%d %s\n", tt->types[i].length,  tt->types[i].name);
	}
}


void type_check_function(Node *function, Type_Table *tt){
	if(function->type != TYPE_FUNCTION){
		printf("Unable to type check node because function expexted\n");
		print_node(NULL, function);
		exit(1);
	}
	char *function_name = CHILD(function, 0)->value;

	// Type check parameters
	Node *parameters = CHILD(function, 1);
	Array_Iter pat = make_array_iter(&parameters->nodes);
	Node *parameter = NULL;
	while((parameter = (Node *)next_item(&pat)) != NULL){
		Node *type = CHILD(parameter, 1);
		//printf("Checking type '%s'\n", type->value);
		if(find_type(tt, type->value) == NULL){
			printf("In function '%s', type '%s' has not been defined\n", function_name, type->value);
			exit(1);
		}
	}
	
	// Type check return type
	Node *return_type = CHILD(function, 2);
	if(find_type(tt, return_type->value	) == NULL){
		printf("In function '%s', return type '%s' hasn't been defined\n", function_name, return_type->value);
		exit(1);
	}

	if(function->flags & FLAG_EXTERNAL){
		return;
	}

	// Type check block lines
	Node *block = CHILD(function, 3);
	Array_Iter bat = make_array_iter(&block->nodes);
	Node *line = NULL;

	unsigned int has_return = 0;
	while((line = (Node *)next_item(&bat)) != NULL){
		if( line->type == TYPE_DECL ){
			if( find_type(tt, CHILD(line, 1)->value) == NULL){
				printf("In function '%s', declaration 'var %s %s' does not contain a defined type\n", function_name, CHILD(line, 0)->value, CHILD(line, 1)->value);
				exit(1);
			}
		}else if(line->type == TYPE_RETURN){
			has_return = 1;
		}else{
			// Implement ME!!
		}
	}
	// This just decides if the function returns based on if it has seen a return already,
	// obviously not enough though. Will have to figure out code path analysis to make sure
	// all code paths return, and probably insert one if neeeded.
	if(has_return == 0){
		printf("Function %s does not have a return statement\n", function_name);
	}
}

