#include "scope.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Scope* create_scope(Scope *parent){
	Scope *s = (Scope *)malloc(sizeof(Scope));
	s->parent = parent;
	s->variable_count = 0;
	s->variable_cap = 64;
	s->variables = (Variable *)malloc(sizeof(Variable)*s->variable_cap);

	s->offset = 0;

	return s;
}

void grow_variable_table(Scope *s){
	s->variable_cap *= 2;
	s->variables = (Variable *)realloc(s->variables, sizeof(Variable)*s->variable_cap);
}


Scope_Iter make_scope_iter(Scope *s){
	Scope_Iter si;
	si.s = s;
	si.current = 0;
	return si;
}

Variable* next(Scope_Iter *si){
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
Variable* current(Scope_Iter si){
	return si.s->variables+si.s->variable_count;
}



Variable* find_variable(Scope *s, const char *var_name){
	if(s->variable_count == 0){
		return NULL;
	}
	Scope_Iter si = make_scope_iter(s);
	Variable *v = next(&si);
	while(v != NULL){
		if(strcmp(v->name, var_name) == 0){
			return v;
		}
		v = next(&si);	
	}
	return NULL;
}	

void add_variable(Scope *s, const char *var_name, Type *t){
	if(s->variable_count == s->variable_cap){
		grow_variable_table(s);
	}
	Variable v;
	
	//VERY TEMPORARY DO ACTUAL SIZE STUFF
	v.offset = s->offset++;

	memcpy((void *)v.name, (void *)var_name, strlen(var_name)+1);
	s->variables[s->variable_count] = v;
	s->variable_count++;


}

