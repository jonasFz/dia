#ifndef _H_NAME
#define _H_NAME

typedef struct Name{
	char *data; //This includes new line at the end
	int length; //Length of the string, WITHOUT new line, maybe this is just a disaster waiting to happen?
}Name;

// Reserves new space
Name make_name(char *data);
int check_equal(Name a, Name b);

void destroy_name(Name *name);
#endif
