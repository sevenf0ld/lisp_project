#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h> //read editable input from prompt
//#include <editline/history.h> //record the inputs for retrieval

//static char input[2048];

int main(int argc, char **argv)
{
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
	
	return 0;
}
