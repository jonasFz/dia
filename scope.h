#ifndef _H_SCOPE
#define _H_SCOPE

#include "array.h"

typedef struct Identifier{
	char *name;
	unsigned int length;
	char is_function;
}Identifier;

typedef struct Scope Scope;

struct Scope {
	Scope *parent;
	Array identifiers;
};

Scope make_scope();
//True if succesfull false if failed
int bind_identifier(Scope *scope, const char* name, unsigned int length, char is_function);
//Null if not found
Identifier* lookup_identifier(Scope *scope, const char* value);
Scope scope_up(Scope *scope);
Scope scope_down(Scope *scope);


#endif
