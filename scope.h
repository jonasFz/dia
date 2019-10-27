#ifndef _H_SCOPE
#define _H_SCOPE

#include "type.h"

typedef struct Variable {
	char name[64];
	Type *type;
	unsigned int offset;
} Variable;

typedef struct Scope Scope;
struct Scope {
	Scope *parent;

	Variable *variables;
	int variable_count;
	int variable_cap;

	unsigned int offset;
};

typedef struct Scope_Iter{
	Scope *s;
	int current;
} Scope_Iter;

Scope_Iter make_scope_iter(Scope *s);

Variable* next(Scope_Iter* si);
Variable* current(Scope_Iter si);

Scope* create_scope(Scope *parent);

void add_variable(Scope *s, const char *var_name, Type *t);
Variable* find_variable(Scope *s, const char *var_name);

#endif
