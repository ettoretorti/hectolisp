#include "SchemeSecret.h"

#include <assert.h>

static Expr* number(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("No args passed to number? (expected 1)");

	Expr* fst = scm_car(args);
	Expr* rst = scm_cdr(args);

	if(rst != EMPTY_LIST) return scm_mk_error("Too many args passed to number? (expected 1)");

	return (scm_is_int(fst) || scm_is_real(fst)) ? TRUE : FALSE;
}

static Expr* integer(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("No args passed to integer? (expected 1)");

	Expr* fst = scm_car(args);
	Expr* rst = scm_cdr(args);

	if(rst != EMPTY_LIST) return scm_mk_error("Too many args passed to integer? (expected 1)");

	return scm_is_int(fst) ? TRUE : FALSE;
}

static Expr* real(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("No args passed to real? (expected 1)");

	Expr* fst = scm_car(args);
	Expr* rst = scm_cdr(args);

	if(rst != EMPTY_LIST) return scm_mk_error("Too many args passed to real? (expected 1)");

	return scm_is_real(fst) ? TRUE : FALSE;
}

static Expr* exact(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("No args passed to exact? (expected 1)");

	Expr* fst = scm_car(args);
	Expr* rst = scm_cdr(args);

	if(rst != EMPTY_LIST) return scm_mk_error("Too many args passed to exact? (expected 1)");
	if(number(args) != TRUE) return scm_mk_error("Argument to exact? is not a number");

	return scm_is_int(fst) ? TRUE : FALSE;
}

static Expr* inexact(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("No args passed to inexact? (expected 1)");

	Expr* fst = scm_car(args);
	Expr* rst = scm_cdr(args);

	if(rst != EMPTY_LIST) return scm_mk_error("Too many args passed to inexact? (expected 1)");
	if(number(args) != TRUE) return scm_mk_error("Argument to inexact? is not a number");

	return scm_is_real(fst) ? TRUE : FALSE;
}

static Expr* add(Expr* args) {
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


#define mk_ff(name, ptr) static Expr name = { .tag = ATOM, .atom = { .type = FFUNC, .ffptr = ptr }, .protect = true, .mark = true }

mk_ff(NUMBER, number);
mk_ff(INTEGER, integer);
mk_ff(REALL, real);
mk_ff(EXACT, exact);
mk_ff(INEXACT, inexact);

mk_ff(ADD, add);

#define bind_ff(name, oname) scm_env_define(BASE_ENV, scm_mk_symbol(name), &oname)

void scm_init_func() {
	bind_ff("number?", NUMBER);
	bind_ff("integer?", INTEGER);
	bind_ff("real?", REALL);
	bind_ff("exact?", EXACT);
	bind_ff("inexact?", INEXACT);

	bind_ff("+", ADD);
}
