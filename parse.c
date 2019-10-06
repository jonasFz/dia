#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

const char *TYPES[] = {
	"undefined",
	"function",
	"block",
	"statement",
	"ident",
	"assignment",
	"integer",
	"expression",
	"operator",
	"decl",
	"global"
};

/*This is a dangerous function*/
const char *decode_type(unsigned int type){
	return TYPES[type];
}

void create_node_list(node_list *nl){
	nl->len = 0;
	nl->cap = 8;

	nl->nodes = (node *)malloc(nl->cap*sizeof(node));
}


// CAUTION that *n down there might have consequences.
void append_node(node *p, node *n){
	node_list nl = p->nodes;
	if (nl.len == nl.cap){
		nl.nodes = (node *)realloc(nl.nodes, nl.cap*2*sizeof(node));
		nl.cap *= 2;
	}
	nl.nodes[nl.len++] = *n;
	p->nodes = nl;
}

node* create_node(parser *p, int index, int length){
	node *n = (node *)malloc(sizeof(node));
	n->index = index;
	n->length = length;

	n->value = (char *) malloc(sizeof(char) * (length+1));
	memcpy((void *)n->value, (void *)p->src+index, sizeof(char) * length);
	n->value[length] = '\0';

	n->precedence = 0;
	//Setting precedence to its highest possible value. Signifies precedence is not relevent for this node.
	n->precedence -= 1;

	n->type = TYPE_UNDEFINED;

	n->scope = NULL;

	create_node_list(&n->nodes);

	return n;
}

char *load_file(char *);
parser make_parser(char *src_path){
	parser p;
	p.src = load_file(src_path);
	p.cur = 0;
	p.off = 0;

	p.state = STATE_PARSING;

	return p;
}

/*Just wrapping exit until I decided what actually needs to be done*/
void fail_hard(){
	exit(0);
}



node* claim_type(parser *p, unsigned int type){

	node *n = create_node(p, p->cur, p->off);
	n->type = type;

	p->cur +=  p->off;
	p->off = 0;

	return n;
}

node* claim(parser *p){
	return claim_type(p, TYPE_UNDEFINED);
}
void print_indent(int indent){
	while (indent != 0){
		printf(" ");
		indent--;
	}
}

void __print_node(parser *p, node *n, int indent){
	print_indent(indent);
	printf("%s:%s", decode_type(n->type), n->value);
	/*for(int i = 0; i < n->length; i++){
		printf("%c", p->src[n->index+i]);
	}*/
	if (n->nodes.len>0){
		printf("[\n");
		for (int i = 0; i < n->nodes.len; i++){
			__print_node(p, &n->nodes.nodes[i], indent + 1);
		}
		print_indent(indent);
		printf("]\n");
	}else{
		printf("[]\n");
	}

}
void print_node(parser *p, node *n){
	__print_node(p, n, 0);
}

/* Maybe this should be inlined, or a macro */
void eat_spaces(parser *p){
	while(CUR(p) == ' ' || CUR(p) == '\n' || CUR(p) == '\t'){
		p->cur++;
	}
	p->off = 0;
}

int accept(parser *p, char *pattern){
	int i = 0;
	while(pattern[i] != '\0'){
		if (pattern[i] != p->src[p->cur+i]){
			return 0;
		}
		i++;
	}
	p->off = i;

	return 1;
}

void ignore_current(parser *p){
	p->cur += p->off;
	p->off = 0;
}

//These accept funtions are assuming that the initial offset equals zero
int accept_ident(parser *p){
	//ident cannot start with a number
	if (!ALPHA(CUR(p))){
		return p->off;
	}

	while((CUR(p) >= 'a' && CUR(p) <='z') || (CUR(p) >= 'A' && CUR(p) <= 'Z') || CUR(p) == '_' || (CUR(p)>='0' && CUR(p) <= '9')){
		p->off++;
	}
	return p->off;
}

int accept_digit(parser *p){
	while(CUR(p) >= '0' && CUR(p) <='9'){
		p->off++;
	}	
	return p->off;
}


//Incomplete
//precedence choices match c for now, until I figure out what I need
int accept_operator(parser *p, unsigned int *precedence){
	if (CUR(p) == '='){
		p->off++;
		*precedence = 16;
	}else if (CUR(p) == '*'|| CUR(p) == '/'){
		p->off++;
		*precedence = 5;
	}else if (CUR(p) == '+' || CUR(p) == '-'){
		p->off++;
		*precedence = 6;
	}else if (accept(p, ":=")){
		*precedence = 16;
	}
	return p->off;
}

node* parse_digit(parser *p){
	eat_spaces(p);
	if (accept_digit(p)){
		return claim_type(p, TYPE_INTEGER);
	}
	return NULL;
}

node* parse_ident(parser *p){
	eat_spaces(p);
	if (accept_ident(p)){
		return claim_type(p, TYPE_IDENT);
	}

	return NULL;
}

node* parse_operator(parser *p){
	eat_spaces(p);
	unsigned int precedence = 0;
	if (accept_operator(p, &precedence)){
		node *ret = claim_type(p, TYPE_OPERATOR);
		ret->precedence = precedence;
		return ret;
	}
	return NULL;
}

node* parse_decl(parser *p){
	eat_spaces(p);

	if (!accept(p, "var")){
		return NULL;	
	}
	ignore_current(p);

	node* ret = claim_type(p, TYPE_DECL);

	node *ident = parse_ident(p);
	node *type = parse_ident(p);

	if(type == NULL){
		printf("No type given when declaring '%s'\n'", ident->value);
		exit(1);
	}
	append_node(ret, ident);
	append_node(ret, type);
	
	return ret;
}
/*
node* parse_statement(parser *p){
	eat_spaces(p);

	node *lhs = NULL;
	if (accept(p, "var")){
		ignore_current(p);
		lhs = parse_decl(p);
			
	}else{
		parse_expression()
	}
	return NULL;
}
*/
node* parse_operand(parser *p){

	node *ret = parse_decl(p);
	if (ret) return ret;

	ret = parse_ident(p);
	if (ret) return ret;

	ret = parse_digit(p);
	if (ret) return ret;

	return NULL;
}

int compare_precedence(node *a, node *b){
	if (a->precedence < b->precedence){
		return 1;
	}else if (a->precedence > b->precedence){
		return -1;
	}else{
		return 0;
	}
}

node* parse_expression(parser *p){
	node *nstack[128];
	int nsp = 0;

	node *ostack[128];
	int osp = 0;

	eat_spaces(p);
	while(!accept(p, ";")){
		node *thing = parse_operator(p);
		if (thing){
			if(osp > 0){
				if(compare_precedence(ostack[osp-1], thing) >= 0){
					node *top = ostack[--osp];
					node *r = nstack[--nsp];
					node *l = nstack[--nsp];
					append_node(top, l);
					append_node(top, r);
					nstack[nsp++] = top;
				}
			}
			ostack[osp++] = thing;
		}else{
			node *operand = parse_operand(p);
			nstack[nsp++] = operand;
		}
		eat_spaces(p);
	}
	ignore_current(p); //Throwing out the semicolon
	while(osp !=0){
		node *top = ostack[--osp];
		node *r = nstack[--nsp];
		node *l = nstack[--nsp];
		append_node(top, l);
		append_node(top, r);
		nstack[nsp++] = top;
	}
	if (nsp != 1){
		printf("Strange number of things left on stack when parsing expression %d\n", nsp);
		fail_hard();
	}
	return nstack[0];
}

node* parse_block(parser *p){
	
	node *block = claim_type(p, TYPE_BLOCK);
	
	eat_spaces(p);
	if (!accept(p, ":")){
		printf("Expecting a ':' at begining of block\n");
		p->state = STATE_ERROR;
		fail_hard();
	}
	ignore_current(p);
	
	eat_spaces(p);

	while(!accept(p, "end")){
		append_node(block, parse_expression(p));	
		eat_spaces(p);
	}
	ignore_current(p);
	return block;


}

node* parse_function(parser *p){
	eat_spaces(p);
	
	if(!accept(p, "function")){
		printf("Expected a 'function' keyword\n");
		p->state = STATE_ERROR;
		fail_hard();
	}
	ignore_current(p);

	node *n = claim_type(p, TYPE_FUNCTION);

	eat_spaces(p);

	accept_ident(p);	
	
	node *ident = parse_ident(p);

	append_node(n, ident);
	append_node(n, parse_block(p));


	return n;
}
node *parse_global(parser *p){
	node *global = claim_type(p, TYPE_GLOBAL);

	eat_spaces(p);
	while(!DONE(p)){
		append_node(global, parse_function(p));
		eat_spaces(p);
	}
	return global;
}

