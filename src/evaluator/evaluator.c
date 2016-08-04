#include "Scheme.h"
#include "SchemeSecret.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	if(argc <= 1) {
		printf("Please provide a scheme expressions to evaluate as command line args.\n");
		return -1;
	}
	
	scm_init();
	
	for(int i = 1; i < argc; i++) {
		Expr* e = scm_read(argv[i]);
		Expr* e1 = scm_eval(e);
		char* s = scm_print(e1);
		printf("%s\n", s);
		free(s);
	}
	
	scm_reset();

	return 0;
}
