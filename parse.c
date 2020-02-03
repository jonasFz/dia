#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "array.h"

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
	"global",
	"parameters",
	"call",
	"call_parameters",
	"if_statement",
	"return",
	"conditional",
	"else",
	"else if"
};

/*This is a dangerous function*/
const char *decode_type(unsigned int type){
	return TYPES[type];
}

// CAUTION that *n down there might have consequences.
void append_node(Node *p, Node *n){
	add_item(&p->nodes, (void *)n);
}

void fail_parse(Parser *p, char *message){
	printf("%s\n", message);
	Source_Location location = p->current_location;
	char *line = p->src + location.pointer;
	while(1){
		if(line[0] == '\n') break;
		putchar(line[0]);
		line++;
	}

	printf("        [%d:%d] ", location.line + 1, location.index + 1);
	printf("\n");
	for(int i = 0; i < location.index; i++){
		putchar('-');
	}
	printf("^\n");
}

void set_node_value(Node *n, char * value){
	int length = strlen(value);
	n->value = (char *)malloc(sizeof(char) * length+1);
	memcpy((void *)n->value, (void *)value, sizeof(char)*length);
	n->value[length] = '\0';
}

Node* create_node(Parser *p, int index, int length){
	Node *n = (Node *)malloc(sizeof(Node));
	n->index = index;
	n->length = length;

	n->value = (char *) malloc(sizeof(char) * (length+1));
	memcpy((void *)n->value, (void *)p->src+index, sizeof(char) * length);
	n->value[length] = '\0';

	n->precedence = 0;
	//Setting precedence to its highest possible value. Signifies precedence is not relevent for this node.
	n->precedence -= 1;

	n->type = TYPE_UNDEFINED;
	n->nodes = make_array(sizeof(Node));

	return n;
}

char *load_file(char *);
Parser make_parser(char *src_path){
	Parser p;
	p.src = load_file(src_path);
	p.cur = 0;
	p.off = 0;

	p.state = STATE_PARSING;

	p.current_location.line = 0;
	p.current_location.index = 0;
	p.current_location.pointer = 0;

	return p;
}

/*Just wrapping exit until I decided what actually needs to be done*/
void fail_hard(void){
	exit(0);
}

void advance_cursor(Parser *p, int amount){
	p->off = 0;
	for(int i = 0; i < amount; i++){
		p->current_location.index++;
		if(CUR(p) == '\n'){
			p->current_location.pointer += p->current_location.index;
			p->current_location.line++;
			p->current_location.index = 0;
		}
	}
	p->cur += amount;
}

void advance(Parser *p){
	advance_cursor(p, p->off);
	p->off = 0;
}

Node* claim_type(Parser *p, unsigned int type){

	Node *n = create_node(p, p->cur, p->off);
	n->type = type;
	n->flags = 0;

	n->source_location = p->current_location;

	advance(p);

	return n;
}

Node* claim(Parser *p){
	return claim_type(p, TYPE_UNDEFINED);
}

void print_indent(int indent){
	while (indent != 0){
		printf("-");
		indent--;
	}
}

void __print_node(Parser *p, Node *n, int indent, int count){
	print_indent(indent);
	if(!(n->flags & FLAG_EXTERNAL)){	
		printf("%d: %s:%s", count, decode_type(n->type), n->value);
	}else{
		printf("%d: <external> %s:%s", count, decode_type(n->type), n->value);
	}
	if (n->nodes.item_count > 0){
		printf("[\n");
		for (int i = 0; i < n->nodes.item_count; i++){
			__print_node(p, (Node *)INDEX(n->nodes, i), indent + 1, i);
		}
		print_indent(indent);
		printf("]\n");
	}else{
		printf("[]\n");
	}
}

void print_node(Parser *p, Node *n){
	__print_node(p, n, 0, 0);
}

/* Maybe this should be inlined, or a macro */
void eat_spaces(Parser *p){
	while(CUR(p) == ' ' || CUR(p) == '\n' || CUR(p) == '\t'){
		advance_cursor(p, 1);
	}
	p->off = 0;
}

int accept(Parser *p, char *pattern){
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

void ignore_current(Parser *p){
	advance(p);
}

//These accept funtions are assuming that the initial offset equals zero
int accept_ident(Parser *p){
	//ident cannot start with a number
	if (!ALPHA(CUR(p))){
		return p->off;
	}

	while((CUR(p) >= 'a' && CUR(p) <='z') || (CUR(p) >= 'A' && CUR(p) <= 'Z') || CUR(p) == '_' || (CUR(p)>='0' && CUR(p) <= '9')){
		p->off++;
	}
	return p->off;
}

int accept_digit(Parser *p){
	//Accept a negative if we need to
	if(CUR(p) == '-'){
		p->off++;
		// No space after negative cause it makes things easier
		if(CUR(p) == ' '){
			p->off--;
		}
	}
	while(CUR(p) >= '0' && CUR(p) <='9'){
		p->off++;
	}	
	return p->off;
}

//Incomplete
//precedence choices match c for now, until I figure out what I need
int accept_operator(Parser *p, unsigned int *precedence){
	if (CUR(p) == '='){
		p->off++;
		if(CUR(p) == '='){
			p->off++;
			*precedence = 7;
		}else{
			*precedence = 16;
		}
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

Node* parse_digit(Parser *p){
	eat_spaces(p);
	if (accept_digit(p)){
		return claim_type(p, TYPE_INTEGER);
	}
	return NULL;
}

Node* parse_ident(Parser *p){
	eat_spaces(p);
	if (accept_ident(p)){
		return claim_type(p, TYPE_IDENT);
	}

	return NULL;
}

Node* parse_operator(Parser *p){
	eat_spaces(p);
	unsigned int precedence = 0;
	if (accept_operator(p, &precedence)){
		Node *ret = claim_type(p, TYPE_OPERATOR);
		ret->precedence = precedence;
		return ret;
	}
	return NULL;
}

Node* parse_decl(Parser *p){
	eat_spaces(p);

	if (!accept(p, "var")){
		return NULL;	
	}
	ignore_current(p);

	Node* ret = claim_type(p, TYPE_DECL);

	Node *ident = parse_ident(p);
	Node *type = parse_ident(p);

	if(type == NULL){
		printf("No type given when declaring '%s'\n'", ident->value);
		fail_parse(p, "");
		exit(1);
	}
	append_node(ret, ident);
	append_node(ret, type);
	
	return ret;
}


Node* parse_expression(Parser *);
Node* parse_call_params(Parser *p){
	eat_spaces(p);
	if(!accept(p, "(")){
		return NULL;
	}
	ignore_current(p);
	Node* params = claim_type(p, TYPE_CALL_PARAMS);
	eat_spaces(p);
	if(accept(p, ")")){
		return params;
	}
	while(1){
		eat_spaces(p);
		append_node(params, parse_expression(p));
		if(accept(p, ")")){
			ignore_current(p);
			return params;
		}else if(accept(p, ",")){
			ignore_current(p);
		}else{
			fail_parse(p, "Failed to parse parameter list, expected ',' or ')'");
			p->state = STATE_ERROR;
			fail_hard();
		}
	}
	return NULL;
}

Node* parse_operand(Parser *p){
	Node *ret = parse_decl(p);
	if (ret) return ret;

	ret = parse_ident(p);
	eat_spaces(p);

	//Is this a function call or just a regular identifier
	if (ret){
		Node *params = parse_call_params(p);
		if(params){
			Node *fcall = claim_type(p, TYPE_CALL);
			append_node(fcall, ret);
			append_node(fcall, params);
			return fcall;
		}
		return ret;
	}

	ret = parse_digit(p);

	if (ret) return ret;
	return NULL;
}

int compare_precedence(Node *a, Node *b){
	if (a->precedence < b->precedence){
		return 1;
	}else if (a->precedence > b->precedence){
		return -1;
	}else{
		return 0;
	}
}

Node* parse_expression(Parser *p){
	Node *nstack[128];
	int nsp = 0;

	Node *ostack[128];
	int osp = 0;

	eat_spaces(p);
	int gogo = 1;
	while(gogo){
		Node *operand = parse_operand(p);
		if(operand){
			if(!operand){
				gogo = 0;
				break;
			}
			nstack[nsp++] = operand;
		}else{
			Node *thing = parse_operator(p);
			if(thing == NULL){
				break;
			}
			//printf("The value of thing is %s\n", thing->value);
			if(osp > 0){
				if(compare_precedence(ostack[osp-1], thing) >= 0){
					Node *top = ostack[--osp];
					Node *r = nstack[--nsp];
					Node *l = nstack[--nsp];
					append_node(top, l);
					append_node(top, r);
					nstack[nsp++] = top;
				}
			}
			ostack[osp++] = thing;
		}
		eat_spaces(p);
	}
	while(osp !=0){
		Node *top = ostack[--osp];
		Node *r = nstack[--nsp];
		Node *l = nstack[--nsp];
		append_node(top, l);
		append_node(top, r);
		nstack[nsp++] = top;
	}
	if(nsp == 0){
		ignore_current(p);
		Node *n = claim_type(p, TYPE_INTEGER);
		set_node_value(n, "0");
		return n;
	}else if (nsp != 1){
		print_node(NULL, nstack[nsp-1]);
		fail_parse(p,"");
		printf("Strange number of things left on stack while parsing expression %d\n", nsp);
		fail_hard();
	}
	return nstack[0];
}

Node* parse_block(Parser *p);
Node *parse_if(Parser *p){
	ignore_current(p);
	Node *if_statement = claim_type(p, TYPE_IF); 	
	eat_spaces(p);
	append_node(if_statement, parse_expression(p));
	eat_spaces(p);
	append_node(if_statement, parse_block(p));

	return if_statement;
}
Node* parse_conditional(Parser *p){
	ignore_current(p);

	Node *conditional = claim_type(p, TYPE_CONDITIONAL);
	append_node(conditional, parse_if(p));
	//We don't actually keep the if keyword
	eat_spaces(p);
	while (accept(p, "else if")){
		//Kinda hacky but just retyping an if node
		Node *else_if = parse_if(p);
		else_if->type = TYPE_ELSE_IF;
		append_node(conditional, else_if);
		eat_spaces(p);
	}
	if(accept(p, "else")){
		printf("Do we not get here?\n");
		ignore_current(p);
		Node *else_block = claim_type(p, TYPE_ELSE);
		eat_spaces(p);
		append_node(else_block, parse_block(p));
		append_node(conditional, else_block);
	}
	return conditional;
}

Node* parse_block(Parser *p){
	
	Node *block = claim_type(p, TYPE_BLOCK);

	eat_spaces(p);
	if (!accept(p, "{")){
		fail_parse(p, "Expecting a '{' at begining of block");
		p->state = STATE_ERROR;
		fail_hard();
	}
	ignore_current(p);
	
	eat_spaces(p);

	while(!accept(p, "}")){
		if (accept(p, "if")){
			ignore_current(p);
			
			Node *if_statement = parse_conditional(p); 

			append_node(block, if_statement);
		}else if(accept(p, "return")){
			ignore_current(p);
			Node *ret = claim_type(p, TYPE_RETURN);
			eat_spaces(p);

			Node *expression = parse_expression(p);

			append_node(ret, expression);
			eat_spaces(p);
			
			if(!accept(p, ";")){
				fail_parse(p, "Return should have a semicolon");
				p->state = STATE_ERROR;
				fail_hard();
			}
			ignore_current(p);

			append_node(block, ret);
		}else{
			// Probably want to be more cautios here
			append_node(block, parse_expression(p));
			eat_spaces(p);
			if(!accept(p, ";")){
				fail_parse(p, "Expression should end with a ';'");
				p->state = STATE_ERROR;
				fail_hard();
			}
			ignore_current(p);
		}
		eat_spaces(p);
	}
	ignore_current(p);
	return block;
}

Node* parse_params(Parser *p){
	eat_spaces(p);

	if(!accept(p, "(")){
		fail_parse(p, "Expexted '('");
		p->state = STATE_ERROR;
		fail_hard();
	}
	ignore_current(p);

	Node *n = claim_type(p, TYPE_PARAM_LIST);

	if(!accept(p, ")")){
		while(1){
			eat_spaces(p);	
			append_node(n, parse_decl(p));
			if(accept(p, ")")){
				ignore_current(p);
				break;
			}
			if(accept(p, ",")){
				ignore_current(p);
			}else{
				fail_parse(p, "Expected a ')' or a ','");
				p->state = STATE_ERROR;
				fail_hard();
			}
		}

	}
	ignore_current(p);
	return n;
}

Node* parse_return_type(Parser *p){
	eat_spaces(p);
	if(!accept(p, "->")){
		fail_parse(p, "Expected a '->' before function body");
		p->state=STATE_ERROR;
		fail_hard();
	}
	ignore_current(p);
	return parse_ident(p);
}

Node* parse_function(Parser *p){
	eat_spaces(p);

	int is_external = 0;
	if(accept(p, "external")){
		ignore_current(p);
		is_external = 1;
	}

	eat_spaces(p);	
	if(!accept(p, "function")){
		fail_parse(p, "Expected a 'function' keyword");
		p->state = STATE_ERROR;
		fail_hard();
	}
	ignore_current(p);

	Node *n = claim_type(p, TYPE_FUNCTION);
	if( is_external){
		n->flags |= FLAG_EXTERNAL;
	}

	eat_spaces(p);

	accept_ident(p);

	Node *ident = parse_ident(p);

	append_node(n, ident);
	append_node(n, parse_params(p));
	append_node(n, parse_return_type(p));
	if(is_external == 0){
		append_node(n, parse_block(p));
	}else{
		eat_spaces(p);
		if(!accept(p, ";")){
			fail_parse(p, "External function headers should be followed by a semi colon");
			fail_hard();
		}
		ignore_current(p);
	}
	return n;
}

Node *parse_global(Parser *p){
	Node *global = claim_type(p, TYPE_GLOBAL);

	eat_spaces(p);
	while(!DONE(p)){
		append_node(global, parse_function(p));
		eat_spaces(p);
	}
	return global;
}

