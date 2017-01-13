#include "Scheme.h"
#include "SchemeSecret.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static Expr* display(Expr* args) {
	assert(args);

	if(!scm_list_len(args) == 1) return scm_mk_error("display expects one argument");

	Expr* toPrint = scm_car(args);

	if(scm_is_string(toPrint)) {
		fputs(scm_sval(toPrint), stdout);
	} else if(scm_is_char(toPrint)) {
		fputc(scm_cval(toPrint), stdout);
	} else {
		char* res = scm_print(scm_car(args));

		if(!res) return scm_mk_error("display encountered an error when printing");

		fputs(res, stdout);
		free(res);
	}

	return EMPTY_LIST;
};

static Expr* newline(Expr* args) {
	assert(args);

	if(args != EMPTY_LIST) return scm_mk_error("newline expects no arguments");
	puts("");
	return EMPTY_LIST;
}

#define mk_ff(name, func) Expr name = { .tag = ATOM, .atom = { .type = FFUNC, .ffptr = func }, .protect = 1, .mark = 1 }

mk_ff(DISPLAY, display);
mk_ff(NEWLINE, newline);

#define bind_ff(name, func) scm_env_define(BASE_ENV, scm_mk_symbol(name), &func);

int main(int argc, char** argv) {
	if(argc <= 1) {
		printf("Please provide a scheme expressions to evaluate as command line args.\n");
		return -1;
	}
	
	scm_init();
	
	bind_ff("display", DISPLAY);
	bind_ff("newline", NEWLINE);

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
