#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <editline/readline.h> 
#include "mpc.h"

typedef struct {
	long num;
	int type;
	char *err;
	char *sym;
	struct lval **cell;
	int count;
} lval;

/*	an enumeration constant starts the progression from 0
	enumerated values for the type field					*/
enum {LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR};

/*	a struct for a valid number	*/
lval *lval_num(long x)
{
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_NUM;
	l->num = x;
	return l;
}

/*	a struct for an erroneous value	*/
lval *lval_err(char *s)
{
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_ERR;
	l->err = malloc(strlen(s) + 1); //add null terminator
	strcpy(l->err, s);
	return l;
}

lval *lval_sym(char *s)
{
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_SYM;
	l->sym = malloc(strlen(s) + 1);
	strcpy(l->sym, s);
	return l;
}

lval *lval_sexpr(void)
{
	lval *l = malloc(sizeof(lval));
	l->type = LVAL_SEXPR;
	l->cell = NULL;
	l->count = 0;
}

void lval_del(lval *l)
{
	switch (l->type)
	{
	case LVAL_NUM:
		break;
	case LVAL_ERR:
		free(l->err);
		break;
	case LVAL_SYM:
		free(l->sym);
		break;
	case LVAL_SEXPR:
		for (int i = 0; i < l->count; i++)
			lval_del(l->cell[i]); //checks against the switch cases
		free(l->cell);
		break;
	}

	free(l);
}

lval *lval_read_num(mpc_ast_t *t)
{
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return ((errno == ERANGE) ? lval_err("Invalid number") : lval_num(x));
}

lval *lval_read(mpc_ast_t *t)
{
	if (strstr(t->tag, "num"))
		return lval_read_num(t);
	if (strstr(t->tag, "sym"))
		return lval_sym(t->contents);
	
	lval *l = NULL;
	if (strcmp(t->tag, ">") == 0)
		l = lval_sexpr();
	if (strstr(t->tag, "sexpr"))
		l = lval_sexpr();
	
	for (int i = 0; i < t->children_num; i++)
	{
		if (strcmp(t->children[i]->contents, "(") == 0)
			continue;
		if (strcmp(t->children[i]->contents, ")") == 0)
			continue;
		if (strcmp(t->children[i]->tag, "regex") == 0)
			continue;
		l = lval_add(l, lval_read(t->children[i]));
	}

	return l;
}

lval *lval_add(lval *l, lval *m)
{
	l->count++;
	l->cell = realloc(l->cell, sizeof(lval *) * l->count);
	l->cell[l->count - 1] = m
	
	return l;
}

void lval_print(lval *l);

void lval_expr_print(lval *l, char open, char close)
{
	putchar(open);
	for (int i = 0; i < v->count; i++)
	{
		lval_print(v->cell[i]);
		if (i != (v->count - 1))
			putchar(' ');
	}
	putchar(close);
}

void lval_print(lval *l)
{
	switch (l->type)
	{
	case LVAL_NUM:
		printf("%li", l->num);
		break;
	case LVAL_ERR:
		printf("Error: %s", l->err);
		break;
	case LVAL_SYM:
		printf("%s", l->sym);
		break;
	case LVAL_SEXPR:
		lval_expr_print(l, '(', ')');
		break;
	}
	putchar('\n');
}

/*void lval_print(lval l)
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
}*/

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
	mpc_parser_t *Sexpr = mpc_new("sexpr");
	mpc_parser_t *Sym = mpc_new("sym");
	mpc_parser_t *Num = mpc_new("num");

	/*	sexprs are expresssions in parantheses	*/
	mpca_lang(MPCA_LANG_DEFAULT,
		"\
			prompt: /^/ <expr>* /$/ ;\
			expr: <num> | <sym> | <sexpr> ;\
			sym: '+' | '-' | '*' | '/' | '%' | '^' ;\
			num: /-?[0-9]+(\\.[0-9]+)?/ ;\
			sexpr: '(' <expr>* ')' ;\
		",
		Prompt, Expr, Sym, Num, Sexpr);

	puts("LISP Version 0.0.0.4");
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

	mpc_cleanup(5, Prompt, Expr, Sym, Num, Sexpr);
}
