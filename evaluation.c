#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <editline/readline.h> 
#include "mpc.h"

long eval_op(long x, char *op, long y)
{
	if (strcmp(op, "+") == 0)
		return x + y;
	if (strcmp(op, "-") == 0)
		return x - y;
	if (strcmp(op, "*") == 0)
		return x * y;
	if (strcmp(op, "/") == 0)
		return x / y;
	if (strcmp(op, "%") == 0)
		return x % y;
}

long eval(mpc_ast_t *t)
{
	if (strstr(t->tag, "num"))
		return atoi(t->contents);

	char *op = t->children[1]->contents;
	long x = eval(t->children[2]);

	for (int i = 3; strstr(t->children[i]->tag, "expr"); i++)
		x = eval_op(x, op, eval(t->children[i]));

	return x;
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
			op: '+' | '-' | '*' | '/' | '%' ;\
			num: /-?[0-9]+(\\.[0-9]+)?/ ;\
		",
		Prompt, Expr, Opr, Num);

	puts("LISP Version 0.0.0.2");
	puts("Ctrl+C to kill and Ctrl+Z to suspend\n");

	while (1)
	{
		char *input = readline("prompt> ");
		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Prompt, &r))
		{
			printf("%li\n", eval(r.output));
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
