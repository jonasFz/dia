#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "scope.h"

Scope make_scope(){
	Scope scope;
	scope.parent = NULL;
	scope.identifiers = make_array(sizeof(Identifier));

	return scope;
}

Scope scope_up(Scope *scope){
	Scope ret = make_scope();
	ret.parent = scope;
	return ret;
}

Scope scope_down(Scope *scope){
	assert(scope != NULL);

	Scope ret = *scope->parent;
	free_array(&scope->identifiers);

	return ret;
}

Identifier *check_non_recursive(Scope *scope, const char *name){
	Array_Iter at = make_array_iter(&scope->identifiers);
	Identifier *ident;
	while((ident = (Identifier *)next_item(&at)) != NULL){
		if(strcmp(name, ident->name) == 0){
			return ident;
		}
	}
	return NULL;
}

// Probably shouldn't use recursion so much
Identifier *lookup_identifier(Scope *scope, const char *value){
	if(scope == NULL){
		return NULL;
	}
	Identifier *ident = check_non_recursive(scope, value);
	if(ident == NULL){
		return lookup_identifier(scope->parent, value);
	}

	return ident;
}

int bind_identifier(Scope *scope, const char* name, unsigned int length, char is_function){
	if(check_non_recursive(scope, name) != NULL){
		return 0;
	}
	Identifier ident;
	
	int len = strlen(name);
	ident.name = (char *)malloc(len+1);
	memcpy(ident.name, name, len);
	ident.name[len] = '\0';

	ident.length = length;
	ident.is_function = is_function;

	// Pretty sure it copies?
	add_item(&scope->identifiers, (void *)&ident);

	return 1;
}
