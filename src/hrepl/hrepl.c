#include <stdlib.h>
#include <stdio.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <Scheme.h>

int main() {
	scm_init();

	Expr* curExpr = EMPTY_LIST;
	scm_stack_push(&curExpr);

	while(true) {
		char* line = readline("> ");
		if(!line) break;
		char* str = line;
		bool error = false;
		while(!error && *str != '\0') {
			curExpr = scm_read_inc(str, &str);
			Expr* res = scm_eval(curExpr);
			error = scm_is_error(res);
			char* printed = scm_print(res);

			puts(printed);
			free(printed);
			curExpr = EMPTY_LIST;
		}
		
		if(line[0] != '\0') add_history(line);
		free(line);
	}

	scm_stack_pop(&curExpr);
	scm_reset();

	return 0;
}
