#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "type.h"
#include "dsm.h"
#include "array.h"
#include "builder.h"


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
	Name_Table nt = make_name_table();


	build_code(global, &code, &nt);
	show_code(&code);
	printf("-----------------------");
	Row* r = lookup_name(&nt, "main");
	if(r == NULL){
		printf("Your program needs a main function\n");
	}else{
		printf("Main function found at instruction index:  %d\n", r->location);
		Interp interp;
		interpret(&interp, &code, &nt, r->location);
	}

	//emit_function(global->nodes.nodes);

	//print_type_table(tt);

	free(p.src);

	return 0;
}
