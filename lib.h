#ifndef _H_LIB
#define _H_LIB

#include "dsm.h"

typedef void (* External)(Interp*);

int lookup_external_index(const char *name);
External get_external_by_index(unsigned int index);


#endif
