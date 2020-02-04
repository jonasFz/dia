#ifndef _H_SCOPE
#define _H_SCOPE

#include "array.h"

typedef struct Identifier{
	char *name;
	unsigned int length;
	unsigned int location;
	// TODO: make these flags
	char is_function;
	char is_external;
}Identifier;

typedef struct Scope Scope;

struct Scope {
	Scope *parent;
	Array identifiers;
};

Scope make_scope();
//True if succesfull false if failed
int bind_identifier(Scope *scope, const char* name, unsigned int length, unsigned int location, char is_function, char is_external);
//Null if not found
Identifier *get_by_index(Scope *scope, unsigned int index);
Identifier* lookup_identifier(Scope *scope, const char* value);
Scope scope_up(Scope *scope);
Scope scope_down(Scope *scope);
int index_of_name(Scope *scope, const char *name);

void set_location(Scope *scope, const char *name, unsigned int location);

#endif
