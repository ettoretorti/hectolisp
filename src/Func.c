#include "SchemeSecret.h"

#include <assert.h>

Expr* add(Expr* args) {
	assert(args);

	double dbuf = 0.0;
	long long lbuf = 0;
	bool exact = true;

	while(scm_is_pair(args)) {
		Expr* cur = scm_car(args);
		if(scm_is_int(cur)) {
			lbuf += scm_ival(cur);
			dbuf += scm_ival(cur);
		} else if(scm_is_real(cur)) {
			exact = false;
			dbuf += scm_rval(cur);
		} else {
			return scm_mk_error("Wrong type of argument to +");
		}
		args = scm_cdr(args);
	}

	if(args != EMPTY_LIST) {
		return scm_mk_error("args to + aren't a proper list");
	}


	return exact ? scm_mk_int(dbuf) : scm_mk_real(dbuf);
}


static Expr ADD = { .tag = ATOM, .atom = { .type = FFUNC, .ffptr = add }, .protect = true, .mark = true };

void scm_init_func() {
	scm_env_define(BASE_ENV, scm_mk_symbol("+"), &ADD);
}
