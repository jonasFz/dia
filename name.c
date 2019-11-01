#include <string.h>
#include <stdlib.h>

#include "name.h"


Name make_name(char *data){
	Name name;
	name.data = NULL;
	name.length = 0;
	if(!data) return name;

	name.length = strlen(data);

	name.data = (char *)malloc(sizeof(char) * name.length + 1);

	for(int i = 0; i< name.length; i++){
		name.data[i] = data[i];
	}
	name.data[name.length] = '\0';
	
	return name;
}

int check_equal(Name a, Name b){
	if(a.length != b.length){
		return 0;
	}

	for(int i = 0; i < a.length; i++){
		if(a.data[i] != b.data[i]){
			return 0;
		}
	}
	return 1;
}

void destroy_name(Name *name){
	if(name->data != NULL){
		free(name->data);
	}
	name->length = 0;
}
