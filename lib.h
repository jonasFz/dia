#ifndef _H_LIB
#define _H_LIB

#include "builder.h"
#include "dsm.h"

#define EXT_COUNT 1

typedef void (* External)(Interp*);

void register_externals(Name_Table *nt);

External* get_externals();

#endif
