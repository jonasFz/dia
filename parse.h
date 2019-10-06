#ifndef _PARSE_H
#define _PARSE_H

#include "scope.h"


#define STATE_ERROR 	0
#define STATE_DONE	1
#define STATE_PARSING	2


#define CUR(p) p->src[p->cur+p->off]
#define DONE(p) p->src[p->cur] == '\0'

#define TYPE_UNDEFINED	0
#define TYPE_FUNCTION	1
#define TYPE_BLOCK	2
#define TYPE_STATEMENT	3
#define TYPE_IDENT	4
#define TYPE_ASSIGNMENT	5
#define TYPE_INTEGER	6
#define TYPE_EXPRESSION 7
#define TYPE_OPERATOR	8
#define TYPE_DECL	9
#define TYPE_GLOBAL	10

#define L(n) n->nodes.nodes[0]
#define R(n) n->nodes.nodes[1]

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define NUM(c) (c >= '0' && c <= '9') 

const char *decode_type(unsigned int type);

typedef struct parser{
	char *src;
	int cur;
	int off;

	int state;

}parser;


typedef struct node node;


typedef struct node_list{
	int len;
	int cap;

	node *nodes;
}node_list;


struct node{
	unsigned int type;

	char *value;

	int index;
	int length;

	unsigned int precedence;

	node_list nodes;

	scope *scope;
};

parser make_parser(char *);
node* parse_function(parser*);
node* parse_global(parser*);
void print_node(parser*, node*);

#endif
