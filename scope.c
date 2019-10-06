#include "scope.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

scope* create_scope(scope *parent){
	scope *s = (scope *)malloc(sizeof(scope));
	s->parent = parent;
	s->variable_count = 0;
	s->variable_cap = 64;
	s->variables = (variable *)malloc(sizeof(variable)*s->variable_cap);

	return s;
}

void grow_variable_table(scope *s){
	s->variable_cap *= 2;
	s->variables = (variable *)realloc(s->variables, sizeof(variable)*s->variable_cap);
}


scope_iter make_scope_iter(scope *s){
	scope_iter si;
	si.s = s;
	si.current = 0;
	return si;
}

variable* next(scope_iter *si){
	if (si->current >= si->s->variable_count){
		if (si->s->parent != NULL){
			si->s = si->s->parent;
			si->current = 0;
		}else{
			return NULL;
		}
	}
	return si->s->variables + (si->current++);
}
variable* current(scope_iter si){
	return si.s->variables+si.s->variable_count;
}



variable* find_variable(scope *s, const char *var_name){
	if(s->variable_count == 0){
		return NULL;
	}
	scope_iter si = make_scope_iter(s);
	variable *v = next(&si);
	while(v != NULL){
		if(strcmp(v->name, var_name) == 0){
			return v;
		}
		v = next(&si);	
	}
	return NULL;
}	

void add_variable(scope *s, const char *var_name){
	if(s->variable_count == s->variable_cap){
		grow_variable_table(s);
	}
	variable v;
	memcpy((void *)v.name, (void *)var_name, strlen(var_name)+1);
	s->variables[s->variable_count] = v;
	s->variable_count++;

}

