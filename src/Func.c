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

static Expr* boolean(Expr* args) {
	assert(args);
	
	if(scm_list_len(args) != 1) return scm_mk_error("boolean? expects 1 arg");

	return scm_is_bool(scm_car(args)) ? TRUE : FALSE;
}

static Expr* not(Expr* args) {
	assert(args);
	
	if(scm_list_len(args) != 1) return scm_mk_error("not expects 1 arg");

	return scm_is_true(scm_car(args)) ? FALSE : TRUE;
}

#define checknum(x) if(!scm_is_num(x)) return scm_mk_error("= expects only numbers")

static Expr* num_eq(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return TRUE;
	
	Expr* cur = scm_car(args);
	checknum(cur);
	
	bool eq = true;
	bool exact = scm_is_int(cur);
	long long ex;
	double in;

	if(exact) {
		ex = scm_ival(cur);
		in = ex;
	} else {
		in = scm_rval(cur);
		ex = in;
		
		exact = ((double)ex) == in;
	}

	args = scm_cdr(args);

	while(scm_is_pair(args)) {
		cur = scm_car(args);
		checknum(cur);

		if(exact && scm_is_int(cur)) {
			if(ex != scm_ival(cur)) {
				eq = false;
				break;
			}
		} else if(exact) {
			if(in != scm_rval(cur)) {
				eq = false;
				break;
			}
		} else if(scm_is_real(cur)) {
			if(in != scm_rval(cur)) {
				eq = false;
				break;
			}
		} else {
			eq = false;
			break;
		}

		args = scm_cdr(args);
	}

	if(eq && args != EMPTY_LIST) return scm_mk_error("arguments to = aren't a proper list");

	return eq ? TRUE : FALSE;
}

#undef checknum

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

static Expr* pair(Expr* args) {
	assert(args);

	if(scm_cdr(args) != EMPTY_LIST) return scm_mk_error("passed more than 1 arg to pair?");
	
	return scm_is_pair(scm_car(args)) ? TRUE : FALSE;
}

static Expr* car(Expr* args) {
	assert(args);

	if(scm_cdr(args) != EMPTY_LIST) return scm_mk_error("passed more than 1 arg to car");

	Expr* arg = scm_car(args);
	if(!scm_is_pair(arg)) return scm_mk_error("arg to car must be a pair");

	return scm_car(arg);
}

static Expr* cdr(Expr* args) {
	assert(args);

	if(scm_cdr(args) != EMPTY_LIST) return scm_mk_error("passed more than 1 arg to cdr");

	Expr* arg = scm_car(args);
	if(!scm_is_pair(arg)) return scm_mk_error("arg to cdr must be a pair");

	return scm_cdr(arg);
}

static Expr* cons(Expr* args) {
	assert(args);
	
	if(scm_list_len(args) != 2) return scm_mk_error("cons expects 2 args");
	
	Expr* es[2] = { scm_car(args), scm_cadr(args) };
	
	scm_stack_push(&es[0]);
	scm_stack_push(&es[1]);

	Expr* toRet = scm_mk_pair(es[0], es[1]);

	scm_stack_pop(&es[1]);
	scm_stack_pop(&es[0]);

	return toRet ? toRet : OOM;
}


#define mk_ff(name, ptr) static Expr name = { .tag = ATOM, .atom = { .type = FFUNC, .ffptr = ptr }, .protect = true, .mark = true }

mk_ff(NUMBER, number);
mk_ff(INTEGER, integer);
mk_ff(REALL, real);
mk_ff(EXACT, exact);
mk_ff(INEXACT, inexact);

mk_ff(BOOLEAN, boolean);
mk_ff(NOT, not);

mk_ff(NUM_EQ, num_eq);
mk_ff(ADD, add);

mk_ff(PAIRR, pair);
mk_ff(CAR, car);
mk_ff(CDR, cdr);
mk_ff(CONS, cons);

#define bind_ff(name, oname) scm_env_define(BASE_ENV, scm_mk_symbol(name), &oname)

void scm_init_func() {
	bind_ff("number?", NUMBER);
	bind_ff("integer?", INTEGER);
	bind_ff("real?", REALL);
	bind_ff("exact?", EXACT);
	bind_ff("inexact?", INEXACT);

	bind_ff("boolean?", BOOLEAN);
	bind_ff("not", NOT);
	
	bind_ff("=", NUM_EQ);
	bind_ff("+", ADD);

	bind_ff("pair?", PAIRR);
	bind_ff("car", CAR);
	bind_ff("cdr", CDR);
	bind_ff("cons", CONS);
}
