#ifndef _H_BUILDER
#define _H_BUILDER

#include "array.h"
#include "parse.h"

typedef struct Row{
	char *name;
	int location;
	int is_external;
}Row;

typedef struct Name_Table{
	Array names;
} Name_Table;

Name_Table make_name_table();

typedef struct Code Code;

void set_location(Name_Table *nt, char *name, int location);
void register_name(Name_Table *nt, char *name, int is_external);
Row* lookup_name(Name_Table *nt, char *name);
int index_of_name(Name_Table *nt, char *name);
void build_code(Node *node, Code *code, Name_Table * nt);
int get_location_of_name(Name_Table *nt, char *name);

#endif
