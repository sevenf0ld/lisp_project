#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h> //read editable input from prompt
//#include <editline/history.h> //record the inputs for retrieval
#include "mpc.h"

//static char input[2048];

int main(int argc, char **argv)
{
	mpc_parser_t *Prompt = mpc_new("prompt");
	mpc_parser_t *Expr = mpc_new("expr");
	mpc_parser_t *Opr = mpc_new("op");
	mpc_parser_t *Num = mpc_new("num");

	mpca_lang(MPCA_LANG_DEFAULT,
		"\
			prompt: /^/ <op> <expr>+ /$/ ;\
			expr: <num> | /(/ <op> <expr>+ /)/ ;\
			op: '+' | '-' | '*' | '/' ;\
			num: /-?[0-0]+/ ;\
		",
		Prompt, Expr, Opr, Num);

	puts("LISP Version 0.0.0.0");
	puts("Ctrl+C to kill and Ctrl+Z to suspend\n");

	while (1)
	{
		/*fputs("lispy> ", stdout);
		fgets(input, 2048, stdin);*/
		char *input = readline("prompt> ");
		add_history(input);
		printf("Kevin, I don't care about %s\n", input);
		free(input);
	}

	mpc_cleanup(4, Prompt, Expr, Opr, Num);
	
	return 0;
}
