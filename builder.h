#ifndef _H_BUILDER
#define _H_BUILDER

//include "dsm.h"
#include "array.h"
#include "parse.h"

typedef struct Row{
	char *name;
	int location;
}Row;

typedef struct Name_Table{
	Array names;
} Name_Table;

Name_Table make_name_table();

typedef struct Code Code;

void set_location(Name_Table *nt, char *name, int location);
void register_name(Name_Table *nt, char *name);
int lookup_name(Name_Table *nt, char *name);
void build_code(Node *node, Code *code, Name_Table * nt);

#endif
