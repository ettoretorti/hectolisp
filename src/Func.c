#include "SchemeSecret.h"

#include <string.h>
#include <assert.h>

// Numerical predicates

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

// Numerical conversions

static Expr* ex2in(Expr* args) {
	assert(args);

	if(scm_list_len(args) != 1) return scm_mk_error("exact->inexact expects 1 arg");

	Expr* fst = scm_car(args);

	if(scm_is_int(fst)) {
		Expr* toRet = scm_mk_real(scm_ival(fst));
		return toRet ? toRet : OOM;
	} else if(scm_is_real(fst)) {
		return fst;
	} else {
		return scm_mk_error("exact->inexact expects a number");
	}
}

static Expr* in2ex(Expr* args) {
	assert(args);

	if(scm_list_len(args) != 1) return scm_mk_error("inexact->exact expects 1 arg");

	Expr* fst = scm_car(args);

	if(scm_is_int(fst)) {
		return fst;
	} else if(scm_is_real(fst)) {
		Expr* toRet = scm_mk_int(scm_rval(fst));
		return toRet ? toRet : OOM;
	} else {
		return scm_mk_error("inexact->exact expects a number");
	}

}

// Character conversions

static Expr* chr2int(Expr* args) {
	assert(args);
	
	if(scm_list_len(args) != 1) return scm_mk_error("char->integer expects 1 arg");

	Expr* fst = scm_car(args);

	if(!scm_is_char(fst)) return scm_mk_error("char->integer expects a character");

	return scm_mk_int(scm_cval(fst));
}

static Expr* int2chr(Expr* args) {
	assert(args);
	
	if(scm_list_len(args) != 1) return scm_mk_error("integer->char expects 1 arg");

	Expr* fst = scm_car(args);

	if(!scm_is_int(fst)) return scm_mk_error("integer->char expects an integer");
	
	long long v = scm_ival(fst);
	
	if(!(0 <= v && v < 256)) return scm_mk_error("argument to integer->char is out of range");

	return scm_mk_char((char)v);

}

// Boolean operations

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

// Equality

static Expr* eq(Expr* args) {
	assert(args);

	if(scm_list_len(args) != 2) return scm_mk_error("eq? expects 2 args");

	return scm_car(args) == scm_cadr(args) ? TRUE : FALSE;
}

static Expr* num_eq(Expr*);

static Expr* eqv(Expr* args) {
	assert(args);

	if(scm_list_len(args) != 2) return scm_mk_error("eqv? expects 2 args");
	
	Expr* fst = scm_car(args);
	Expr* snd = scm_cadr(args);

	if(fst == snd) return TRUE;
	if(scm_is_pair(fst) || scm_is_pair(snd)) return FALSE;
	if(scm_is_closure(fst) || scm_is_closure(snd)) return FALSE;
	if(scm_is_num(fst) && scm_is_num(snd)) return num_eq(args);
	if(scm_is_string(fst) && scm_is_string(snd) && strcmp(scm_sval(fst), scm_sval(snd)) == 0) return TRUE;

	return FALSE;
}


// Numerical predicates

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

// Numerical operations

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


	return exact ? scm_mk_int(lbuf) : scm_mk_real(dbuf);
}

static Expr* sub(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("no arguments passed to - (expected at least 1)");

	// unary case
	if(scm_cdr(args) == EMPTY_LIST) {
		Expr* v = scm_car(args);

		if(scm_is_int(v)) return scm_mk_int(-scm_ival(v));
		if(scm_is_real(v)) return scm_mk_int(-scm_rval(v));

		return scm_mk_error("wrong type of argument to -");
	}

	Expr* first = scm_car(args);
	if(!scm_is_num(first)) return scm_mk_error("wrong type of argument to -");

	bool exact = scm_is_int(first);
	double dbuf = exact ? scm_ival(first) : scm_rval(first);
	long long lbuf = exact ? scm_ival(first) : 0;

	args = scm_cdr(args);

	while(scm_is_pair(args)) {
		Expr* cur = scm_car(args);
		if(scm_is_int(cur)) {
			lbuf -= scm_ival(cur);
			dbuf -= scm_ival(cur);
		} else if(scm_is_real(cur)) {
			exact = false;
			dbuf -= scm_rval(cur);
		} else {
			return scm_mk_error("Wrong type of argument to +");
		}
		args = scm_cdr(args);
	}

	if(args != EMPTY_LIST) {
		return scm_mk_error("args to + aren't a proper list");
	}


	return exact ? scm_mk_int(lbuf) : scm_mk_real(dbuf);
}

// Pair operations

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

// Procedure operations

static Expr* procedure(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("procedure? expects an argument");
	if(scm_cdr(args) != EMPTY_LIST) return scm_mk_error("procedure? expects only 1 argument");

	Expr* fst = scm_car(args);

	return fst->tag == CLOSURE || (fst->tag == ATOM && fst->atom.type == FFUNC) ? TRUE : FALSE;
}

static Expr* p_procedure(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("primitive-procedure? expects an argument");
	if(scm_cdr(args) != EMPTY_LIST) return scm_mk_error("primitive-procedure? expects only 1 argument");

	Expr* fst = scm_car(args);

	return (fst->tag == ATOM && fst->atom.type == FFUNC) ? TRUE : FALSE;
}

static Expr* c_procedure(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("compound-procedure? expects an argument");
	if(scm_cdr(args) != EMPTY_LIST) return scm_mk_error("compound-procedure? expects only 1 argument");

	Expr* fst = scm_car(args);

	return fst->tag == CLOSURE ? TRUE : FALSE;
}

static Expr* c_args(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("closure-args expects an argument");
	if(scm_cdr(args) != EMPTY_LIST) return scm_mk_error("closure-args expects only 1 argument");

	Expr* fst = scm_car(args);

	if(fst->tag != CLOSURE) return scm_mk_error("argument to closure-args is not a closure");

	return scm_closure_args(fst);
}

static Expr* c_code(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("closure-code expects an argument");
	if(scm_cdr(args) != EMPTY_LIST) return scm_mk_error("closure-acode expects only 1 argument");

	Expr* fst = scm_car(args);

	if(fst->tag != CLOSURE) return scm_mk_error("argument to closure-code is not a closure");

	return scm_closure_body(fst);
}

static Expr* gc(Expr* args) {
	assert(args);

	if(args != EMPTY_LIST) return scm_mk_error("gc expects no arguments");

	scm_gc();

	return EMPTY_LIST;
}

static Expr* free_mem(Expr* args) {
	assert(args);

	if(args != EMPTY_LIST) return scm_mk_error("gc expects no arguments");
	
	Expr* toRet = scm_mk_int(scm_gc_free_objects());

	return toRet ? toRet : OOM;
}

static Expr* error(Expr* args) {
	assert(args);

	if(args == EMPTY_LIST) return scm_mk_error("generic error");

	if(scm_list_len(args) != 1) return scm_mk_error("error expects zero or one arguments)");

	Expr* msg = scm_car(args);

	if(!scm_is_string(msg)) return scm_mk_error("error expects a string as its argument");

	return scm_mk_error(scm_sval(msg));
}

#define mk_ff(name, ptr) static Expr name = { .tag = ATOM, .atom = { .type = FFUNC, .ffptr = ptr }, .protect = true, .mark = true }

mk_ff(NUMBER, number);
mk_ff(INTEGER, integer);
mk_ff(REALL, real);
mk_ff(EXACT, exact);
mk_ff(INEXACT, inexact);

mk_ff(EX2IN, ex2in);
mk_ff(IN2EX, in2ex);
mk_ff(CHR2INT, chr2int);
mk_ff(INT2CHR, int2chr);

mk_ff(BOOLEAN, boolean);
mk_ff(NOT, not);

mk_ff(EQ, eq);
mk_ff(EQV, eqv);

mk_ff(NUM_EQ, num_eq);
mk_ff(ADD, add);
mk_ff(SUB, sub);

mk_ff(PAIRR, pair);
mk_ff(CAR, car);
mk_ff(CDR, cdr);
mk_ff(CONS, cons);

mk_ff(PROC, procedure);
mk_ff(P_PROC, p_procedure);
mk_ff(C_PROC, c_procedure);
mk_ff(C_ARGS, c_args);
mk_ff(C_CODE, c_code);

mk_ff(GC, gc);
mk_ff(FREE_M, free_mem);
mk_ff(ERRORF, error);

#define bind_ff(name, oname) scm_env_define(BASE_ENV, scm_mk_symbol(name), &oname)

void scm_init_func() {
	bind_ff("number?", NUMBER);
	bind_ff("integer?", INTEGER);
	bind_ff("real?", REALL);
	bind_ff("exact?", EXACT);
	bind_ff("inexact?", INEXACT);

	bind_ff("exact->inexact", EX2IN);
	bind_ff("inexact->exact", IN2EX);
	bind_ff("char->integer", CHR2INT);
	bind_ff("integer->char", INT2CHR);

	bind_ff("boolean?", BOOLEAN);
	bind_ff("not", NOT);

	bind_ff("eq?", EQ);
	bind_ff("eqv?", EQV);
	
	bind_ff("=", NUM_EQ);
	bind_ff("+", ADD);
	bind_ff("-", SUB);

	bind_ff("pair?", PAIRR);
	bind_ff("car", CAR);
	bind_ff("cdr", CDR);
	bind_ff("cons", CONS);

	bind_ff("gc", GC);
	bind_ff("free-mem", FREE_M);
	bind_ff("error", ERRORF);

	bind_ff("procedure?", PROC);
	bind_ff("primitive-procedure?", P_PROC);
	bind_ff("compound-procedure?", C_PROC);
	bind_ff("closure-args", C_ARGS);
	bind_ff("closure-code", C_CODE);
}
