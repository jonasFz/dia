#ifndef _PARSE_H
#define _PARSE_H

#include "array.h"

#define STATE_ERROR 	0
#define STATE_DONE		1
#define STATE_PARSING	2


#define CUR(p) p->src[p->cur+p->off]
#define DONE(p) p->src[p->cur] == '\0'

#define TYPE_UNDEFINED		0
#define TYPE_FUNCTION		1
#define TYPE_BLOCK			2
#define TYPE_STATEMENT		3
#define TYPE_IDENT			4
#define TYPE_ASSIGNMENT		5
#define TYPE_INTEGER		6
#define TYPE_EXPRESSION 	7
#define TYPE_OPERATOR		8
#define TYPE_DECL			9
#define TYPE_GLOBAL			10
#define TYPE_PARAM_LIST 	11
#define TYPE_CALL			12
#define TYPE_CALL_PARAMS	13
#define TYPE_IF				14
#define TYPE_RETURN			15

#define FLAG_EXTERNAL (1<<0)

#define L(n) ((Node *)INDEX(n->nodes, 0))
#define R(n) ((Node *)INDEX(n->nodes, 1))

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define NUM(c) (c >= '0' && c <= '9') 


//Please make sure the list actually contains nodes!
#define CHILD(n, i) ((Node *)(INDEX(n->nodes, i))) 

const char *decode_type(unsigned int type);

typedef struct Source_Location{
	unsigned int line;
	unsigned int index;
	unsigned int pointer;
} Source_Location;

typedef struct Parser{
	char *src;
	int cur;
	int off;

	int state;

	Source_Location current_location;

}Parser;

typedef struct Node Node;

struct Node{
	unsigned int type;
	char *value;
	int index;
	int length;
	unsigned int precedence;
	Array nodes;

	unsigned int flags;
	Source_Location source_location;
};

Parser make_parser(char *);
Node* parse_function(Parser*);
Node* parse_global(Parser*);
void print_node(Parser*, Node*);

#endif
