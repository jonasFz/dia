#ifndef _H_TYPE
#define _H_TYPE

typedef struct type{
	char *name;
	unsigned int length;
}type;

typedef struct type_table{
	type *types;
	unsigned int count;
	unsigned int cap;
} type_table;

type_table* make_type_table();
type* find_type(type_table *tt, const char *name);
void register_type(type_table *tt, const char* name, unsigned int length);

void add_built_in_types(type_table *tt);

#endif
