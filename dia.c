#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "type.h"
#include "dsm.h"
#include "array.h"
#include "builder.h"
#include "scope.h"

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

	fclose(f);
	return data;
}

void type_check_functions(Node* global, Type_Table *tt, Scope *scope){
	if(global->type != TYPE_GLOBAL){
		printf("A node of type global must be passed to 'type_check_functions'\n");
		exit(1);
	}
	Array_Iter at = make_array_iter(&global->nodes);
	Node *node = NULL;
	while((node = (Node *)next_item(&at))!= NULL){
		type_check_function(node, tt);
	}
}

Scope build_scope(Node *global){
	Scope scope;

	if(global->type != TYPE_GLOBAL){
		printf("Can only build code from a node of type GLOBAL\n");
		return scope;
	}
	scope = make_scope();
	Array_Iter nodes = make_array_iter(&global->nodes);
	Node *n = NULL;
	while((n = (Node *)next_item(&nodes)) != NULL){
		if(n->type == TYPE_FUNCTION){
			int is_external = n->flags & FLAG_EXTERNAL;
			bind_identifier(&scope, CHILD(n, 0)->value, 0, 0, 1, is_external);
		}else{
			printf("Only handle functions at the top level currently\n");
		}
	}
	
	return scope;
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


	Code code = make_code();
	//Name_Table nt = build_name_table(global);
	Scope scope = build_scope(global);

	type_check_functions(global, tt, &scope);

	build_code(global, &code, &scope);
	//show_code(&code);
	printf("-----------------------\n");
	Identifier *id = lookup_identifier(&scope, "main");
	if(id == NULL){
		printf("Your program needs a main function\n");
	}else{
		printf("Main function found at instruction index:  %d\n", id->location);
		Interp interp;
		interpret(&interp, &code, &scope, id->location);
	}


	free(p.src);

	return 0;
}
