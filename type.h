#ifndef _H_TYPE
#define _H_TYPE

typedef struct Type{
	char *name;
	unsigned int length;
}Type;

typedef struct Type_Table{
	Type *types;
	unsigned int count;
	unsigned int cap;
} Type_Table;

Type_Table* make_type_table();
Type* find_type(Type_Table *tt, const char *name);
void register_type(Type_Table *tt, const char* name, unsigned int length);

void add_built_in_types(Type_Table *tt);
void print_type_table(Type_Table *tt);

#endif
