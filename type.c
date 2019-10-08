#include <stdlib.h>
#include <string.h>

#include "type.h"

type_table* make_type_table(){
	type_table* tt = (type_table *)malloc(sizeof(type_table));
	tt->types = (type *)malloc(sizeof(type) * 256);
	tt->cap = 256;

	return tt;
}

type* find_type(type_table *tt, const char *name){
	for (int i = 0; i < tt->count; i++){
		if (strcmp(name, tt->types[i].name) == 0){
			return &tt->types[i];
		}
	}
	return NULL;
}

void register_type(type_table *tt, const char *name, unsigned int length){
	if(tt->count == tt->cap){
		tt->types = (type *)realloc(tt->types, tt->cap*2);
		tt->cap *= 2;
	}
	type *t = &tt->types[tt->count++];

	long int name_length = strlen(name);

	t->name = (char *)malloc(sizeof(char) * name_length + 1);
	strncpy(t->name, name, strlen(name));
	t->name[name_length] = '\0';
	t->length = length;
}

void add_built_in_types(type_table *tt){
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
