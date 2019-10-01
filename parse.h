#ifndef _PARSE_H
#define _PARSE_H


#define STATE_ERROR 	0
#define STATE_DONE	1
#define STATE_PARSING	2


#define CUR(p) p->src[p->cur+p->off]

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
	const char *type;

	int index;
	int length;

	unsigned int precedence;

	node_list nodes;
};

parser make_parser(char *);
node* parse_function(parser*);
void print_node(parser*, node*);

#endif
