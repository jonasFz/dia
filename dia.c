#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "type.h"
#include "dsm.h"
#include "array.h"
#include "builder.h"


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
/*
void emit_functions(Code *code, Node *functions){
	
	for (int i = 0; i< functions->nodes.item_count; i++){
		emit_function(code, CHILD(functions, i));
	}
}*/

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
	Name_Table nt = make_name_table();


	build_code(global, &code, &nt);
	
	int start = lookup_name(&nt, "main");
	start = ((Row *)get_item(&nt.names, start))->location;
	if(start == -1){
		printf("Your program needs a main function\n");
	}else{
		printf("Main function found at instruction index:  %d\n", start);
		Interp interp;
		interpret(&interp, &code, &nt, start);
		printf("%d\n", interp.stack[interp.reg[SP]]);
	}

	//emit_function(global->nodes.nodes);

	//print_type_table(tt);

	free(p.src);

	return 0;
}
