#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "type.h"

void verify(type_table * tt, scope *scope, node *n){
	if(n->type == TYPE_GLOBAL){
		n->scope = scope;
		for(int i = 0;i<n->nodes.len;i++){
			verify(tt, scope, &n->nodes.nodes[i]);
		}
	}else if(n->type == TYPE_FUNCTION){
		//Not sure if this is the scope that makes most sense, doesn't really need anyscope
		n->scope = scope;
		node *block = &n->nodes.nodes[1];
		verify(tt, n->scope, block);

	}else if(n->type == TYPE_BLOCK){
		n->scope = create_scope(scope);
		for (int i = 0; i < n->nodes.len; i++){
			verify(tt, n->scope, &n->nodes.nodes[i]);
		}
	}else if(n->type == TYPE_OPERATOR){
		n->scope = scope;
		verify(tt, n->scope, &L(n));
		verify(tt, n->scope, &R(n));
	}else if(n->type == TYPE_DECL){
		if (find_variable(scope, L(n).value)){
			printf("Variable '%s' has already been declared\n", L(n).value);
			exit(1);
		}
		type *t = find_type(tt, R(n).value);
		if(t == NULL){
			printf("Type '%s' has not been defined\n", R(n).value);
			exit(1);
		}
		add_variable(scope, L(n).value, t);

	}else if(n->type == TYPE_IDENT){
		if(!find_variable(scope, n->value)){
			printf("Variable '%s' has not been declared\n", n->value); 
			exit(1);
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

int main(){

	type_table* tt = make_type_table();
	add_built_in_types(tt);

	parser p = make_parser("test.dia");

	node *global = parse_global(&p);
	print_node(&p, global);

	scope *global_scope = create_scope(NULL);
	verify(tt, global_scope, global);

	free(p.src);

	return 0;
}
