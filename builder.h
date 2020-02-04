#ifndef _H_BUILDER
#define _H_BUILDER

#include "array.h"
#include "parse.h"
#include "scope.h"

//Circular dependency when including dms.h, see there for this definiton
typedef struct Code Code;

void build_code(Node *node, Code *code, Scope *scope);

#endif
