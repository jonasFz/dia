#ifndef _PARSE_H
#define _PARSE_H

#include "scope.h"


#define STATE_ERROR 	0
#define STATE_DONE	1
#define STATE_PARSING	2


#define CUR(p) p->src[p->cur+p->off]
#define DONE(p) p->src[p->cur] == '\0'

#define TYPE_UNDEFINED		0
#define TYPE_FUNCTION		1
#define TYPE_BLOCK		2
#define TYPE_STATEMENT		3
#define TYPE_IDENT		4
#define TYPE_ASSIGNMENT		5
#define TYPE_INTEGER		6
#define TYPE_EXPRESSION 	7
#define TYPE_OPERATOR		8
#define TYPE_DECL		9
#define TYPE_GLOBAL		10
#define TYPE_PARAM_LIST 	11
#define TYPE_CALL		12
#define TYPE_CALL_PARAMS	13

#define L(n) n->nodes.nodes[0]
#define R(n) n->nodes.nodes[1]

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define NUM(c) (c >= '0' && c <= '9') 


#define CHILD(n, i) (n->nodes.nodes + i) 

const char *decode_type(unsigned int type);

typedef struct Parser{
	char *src;
	int cur;
	int off;

	int state;

}Parser;


typedef struct Node Node;


typedef struct Node_List{
	int len;
	int cap;

	Node *nodes;
}Node_List;


struct Node{
	unsigned int type;

	char *value;

	int index;
	int length;

	unsigned int precedence;

	Node_List nodes;

	Scope *scope;
};

Parser make_parser(char *);
Node* parse_function(Parser*);
Node* parse_global(Parser*);
void print_node(Parser*, Node*);

#endif
