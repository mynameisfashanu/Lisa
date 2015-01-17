#include "mpc.h"

#ifdef _WIN32

static char buffer[2048];

char* readline(char* prompt)
{

	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer) + 1);
	strcpy(cpy, buffer);
	cpy[strlen(buffer) - 1] = '\0';
	return cpy;
}

void add_history(char* unused) {};

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

enum {LVAL_NUM, LVAL_ERR};

typedef struct
{
	int type;
	long num;
	int err;

}lval;


lval lval_num(long x)
{
	lval v;
	v.type = LVAL_NUM;
	v.num = x;
	return v;
}


lval lval_err(int x)
{
	lval v;
	v.type = LVAL_ERR;
	v.err = x;
	return v;
}


void lval_print(lval v)
{
	switch(v.type)
	{
		case LVAL_NUM :
			printf("%li",v.num); 
		break;
		
		case LVAL_ERR : 
			if(v.err == LERR_DIV_ZERO)
				printf("Error : Divison by zero!");
			if(v.err == LERR_BAD_OP)
				printf("Error : Invalid Operator!");
			if(v.err == LERR_BAD_NUM)
				printf("Error : Invalid Number!");
		break;
	
	}

}

void lval_println(lval v)
{
	lval_print(v);
	putchar('\n');
}

lval eval_op(lval x, char* op, lval y)
{

	if(x.type == LVAL_ERR) {return x;}
	if(y.type == LVAL_ERR) {return y;}
	
	if(strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
	if(strcmp(op, "-") == 0) {	return lval_num(x.num - y.num); }
	if(strcmp(op, "*") == 0) {	return lval_num(x.num * y.num); }
	if(strcmp(op, "/") == 0) 
	{	
		if(y.num == 0)
			return lval_err(LERR_DIV_ZERO);
		else
			return lval_num(x.num / y.num);
	}
	
	return lval_err(LERR_BAD_OP);
}


lval eval(mpc_ast_t* t)
{
	
	if(strstr(t->tag,"number"))
	{
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		if(errno != ERANGE)
			return lval_num(x);
		else
			return lval_err(LERR_BAD_NUM);
	}
	
	char* op = t->children[1]->contents;
	
	lval x = eval(t->children[2]);
	
	int i = 3;
	while(strstr(t->children[i]->tag,"expr"))
	{
		x = eval_op(x,op,eval(t->children[i]));
		i++;
	}
	
	return x;
}

int main(int argc, char** argv)
{
  	
  	mpc_parser_t* Number  	= mpc_new("number");
  	mpc_parser_t* Operator 	= mpc_new("operator");
  	mpc_parser_t* Expr 		= mpc_new("expr");
  	mpc_parser_t* Lisa 		= mpc_new("lisa");
  	
  	
  	
  	mpca_lang(MPCA_LANG_DEFAULT,
  	"																\
  		number 	: /-?[0-9]+/; 										\
  		operator : '+' | '-' | '*' | '/' ;							\
  		expr 		: <number> | '(' <operator> <expr>+ ')';		\
  		lisa 		: /^/ <operator> <expr>+ /$/	;				\
  	",  																
  	Number, Operator, Expr, Lisa);
  	
  	puts("Lisa version 0.0.0.0.3"); 
  	puts("press Ctrl+c to Exit \n"); 
  	
  	while(1)
  	{
  		char* input = readline("Lisa>");
  		add_history(input);
  		
  		mpc_result_t r;
  		
  		if(mpc_parse("stdin",input, Lisa, &r))
  		{
  			lval result = eval(r.output);
  			lval_println(result);
  			mpc_ast_delete(r.output);
  		
  		}
  		else
  		{
  			mpc_err_print(r.error);
  			mpc_err_delete(r.error);
  		}
  		
  		free(input);
  	
  	} 	
  
  return 0;	

}
