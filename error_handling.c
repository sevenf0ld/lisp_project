#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <editline/readline.h> 
#include "mpc.h"

typedef struct {
	long num;
	int type;
	int err;
} lval;

/*	an enumeration constant starts the progression from 0
	enumerated values for the type field					*/
enum {LVAL_NUM, LVAL_ERR};
/*	enumeration for the error field	*/
enum {LERR_ZERO_DIV, LERR_BAD_OPR, LERR_BAD_NUM};

/*	a struct for a valid number	*/
lval lval_num(long x)
{
	lval l;
	l.type = LVAL_NUM; //type is 0
	l.num = x;
	return l;
}
/*	a struct for an erroneous value	*/
lval lval_err(int i)
{
	lval l;
	l.type = LVAL_ERR; //type is 1
	l.err = i;
	return l;
}

void lval_print(lval l)
{
	switch (l.type)
	{
	case LVAL_NUM:
		printf("%li", l.num);
		break;
	case LVAL_ERR:
		if (l.err == LERR_ZERO_DIV)
			printf("Error: Division by 0");
		if (l.err == LERR_BAD_OPR)
			printf("Error: Invalid operator");
		if (l.err == LERR_BAD_NUM)
			printf("Error: Invalid number");
		break;
	}
	putchar('\n');
}

lval eval_op(lval l, char *op, lval m)
{
	/*	return the erroneous struct	*/
	if (l.type == LVAL_ERR)
		return l;
	if (m.type ==  LVAL_ERR)
		return m;

	if (strcmp(op, "+") == 0)
		return lval_num(l.num + m.num);
	if (strcmp(op, "-") == 0)
		return lval_num(l.num - m.num);
	if (strcmp(op, "*") == 0)
		return lval_num(l.num * m.num);
	if (strcmp(op, "/") == 0)
		return ((m.num == 0) ? lval_err(LERR_ZERO_DIV) : lval_num(l.num / m.num));
	if (strcmp(op, "%") == 0)
		return ((m.num == 0) ? lval_err(LERR_ZERO_DIV) : lval_num(l.num % m.num));
	if (strcmp(op, "^") == 0)
		return lval_num(pow(l.num, m.num));
	
	return lval_err(LERR_BAD_OPR);
}

lval eval(mpc_ast_t *t)
{
	if (strstr(t->tag, "num"))
	{
		errno = 0; //for error checking because previous function calls could alter it
		long l = strtol(t->contents, NULL, 10);
		/*	strtol returns the converted number of long type on success
			returns 0 on failure conversion
			returns LONG_MAX or LONG_MIN when the value is out of long range	*/
		return ((errno == ERANGE) ? lval_err(LERR_BAD_NUM) : lval_num(l));
	}

	char *op = t->children[1]->contents; //operator is always the second index
	lval l = eval(t->children[2]);

	for (int i = 3; strstr(t->children[i]->tag, "expr"); i++)
		l = eval_op(l, op, eval(t->children[i]));

	return l;
}

int main(int argc, char **argv)
{
	mpc_parser_t *Prompt = mpc_new("prompt");
	mpc_parser_t *Expr = mpc_new("expr");
	mpc_parser_t *Opr = mpc_new("op");
	mpc_parser_t *Num = mpc_new("num");

	mpca_lang(MPCA_LANG_DEFAULT,
		"\
			prompt: /^/ <op> <expr>+ /$/ ;\
			expr: <num> | '(' <op> <expr>+ ')' ;\
			op: '+' | '-' | '*' | '/' | '%' | '^' ;\
			num: /-?[0-9]+(\\.[0-9]+)?/ ;\
		",
		Prompt, Expr, Opr, Num);

	puts("LISP Version 0.0.0.3");
	puts("Ctrl+C to kill and Ctrl+Z to suspend\n");

	while (1)
	{
		char *input = readline("prompt> ");
		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Prompt, &r))
		{
			lval_print(eval(r.output));
			mpc_ast_delete(r.output);
		}
		else
		{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);
	}

	mpc_cleanup(4, Prompt, Expr, Opr, Num);
}
