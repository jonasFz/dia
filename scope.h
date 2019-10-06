#ifndef _H_SCOPE
#define _H_SCOPE


typedef struct variable {
	char name[64];
} variable;

typedef struct scope scope;
struct scope {
	scope *parent;

	variable *variables;
	int variable_count;
	int variable_cap;
};

typedef struct scope_iter{
	scope *s;
	int current;
} scope_iter;

scope_iter make_scope_iter(scope *s);

variable* next(scope_iter* si);
variable* current(scope_iter si);

scope* create_scope(scope *parent);

void add_variable(scope *s, const char *var_name);
variable* find_variable(scope *s, const char *var_name);

#endif
