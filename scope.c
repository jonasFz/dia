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

void set_location(Scope *scope, const char *name, unsigned int location){
	Identifier *id = NULL;
	if((id = lookup_identifier(scope, name)) == NULL){
		//TODO I'm not sure what we actually want to do in this case, cause we shouldn't have to include everytime
		return;
	}
	id->location = location;
}

int index_of_name(Scope *scope, const char *name){
	Array_Iter at = make_array_iter(&scope->identifiers);
	Identifier *current = NULL;

	int index = 0;
	while((current = (Identifier *)next_item(&at)) != NULL){
		if(strcmp(current->name, name) == 0){
			return index;
		}
		index++;
	}
	return index;

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

//Checks JUST THIS SCOPE, using for function lookup at the global level
Identifier *get_by_index(Scope *scope, unsigned int index){
	return (Identifier *)get_item(&scope->identifiers, index);
}

int bind_identifier(Scope *scope, const char* name, unsigned int length, unsigned int location, char is_function, char is_external){
	if(check_non_recursive(scope, name) != NULL){
		return 0;
	}
	Identifier ident;
	
	int len = strlen(name);
	ident.name = (char *)malloc(len+1);
	memcpy(ident.name, name, len);
	ident.name[len] = '\0';

	ident.length = length;
	ident.location = location;
	ident.is_function = is_function;
	ident.is_external = is_external;

	// Pretty sure it copies?
	add_item(&scope->identifiers, (void *)&ident);

	return 1;
}
