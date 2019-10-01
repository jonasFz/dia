#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"


typedef struct type{
	//32 is probably small, this would be awesome if fixed size, but rather bad on memory, but we will run
	//with it for now, once we compile larger programs we can start to get an idea of how many types, and 
	//how much memory we actually need.
	char name[32];
} type;

typedef struct type_table{
	type *types;
	int length;
	int capacity;
} type_table;

void add_built_ins(type_table *);
type_table make_table(){
	type_table tt;
	tt.length = 0;
	tt.capacity = 64;

	tt.types = (type *)malloc(sizeof(type)*tt.capacity);

	add_built_ins(&tt);
	return tt;
}

void add_type(type_table *tt, const char* type_name){
	type t;
	strcpy(t.name, type_name);
	if (tt->length == tt->capacity){
		tt->types = (type *)realloc(tt->types, sizeof(type)*tt->capacity*2);
		tt->capacity *= 2;
	}
	tt->types[tt->length++] = t;
}

void add_built_ins(type_table *tt){
	add_type(tt, "u8");
	add_type(tt, "s8");
	add_type(tt, "u16");
	add_type(tt, "s16");
	add_type(tt, "u32");
	add_type(tt, "s32");
	add_type(tt, "u64");
	add_type(tt, "s64");
	add_type(tt, "f32");
	add_type(tt, "f64");
}

void show_type_table(type_table *tt){
	for (int i = 0; i< tt->length;i++){
		printf("%s\n", tt->types[i].name);
	}
}

void do_type_check(node *n);

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

	parser p = make_parser("test.dia");

	node *func = parse_function(&p);
	print_node(&p, func);
	
	type_table tt = make_table();

	show_type_table(&tt);

	free(p.src);

	return 0;
}
