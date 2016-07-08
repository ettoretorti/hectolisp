#include "Scheme.h"
#include "SchemeSecret.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	if(argc <= 1) {
		printf("Please provide a scheme expression to read as the first command line arg.\n");
		return -1;
	}
	
	scm_init_mem();
	Expr* e = scm_read(argv[1]);
	char* s = scm_print(e);
	printf("%s\n", s);
	free(s);
	scm_gc();

	return 0;
}
