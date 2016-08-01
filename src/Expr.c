#include "SchemeSecret.h"
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

static Expr _EMPTY_LIST = { .tag = PAIR, .pair = { NULL, NULL }, .protect = true, .mark = true };
Expr* EMPTY_LIST = &_EMPTY_LIST;

static Expr _TRUE = { .tag = ATOM, .atom = { .type = BOOL, .bval = true }, .protect = true, .mark = true };
Expr* TRUE = &_TRUE;

static Expr _FALSE = { .tag = ATOM, .atom = { .type = BOOL, .bval = false }, .protect = true, .mark = true };
Expr* FALSE = &_FALSE;

static Expr _DEFINE = { .tag = ATOM, .atom = { .type = SYMBOL, .sval = "DEFINE" }, .protect = true, .mark = true };
Expr* DEFINE = &_DEFINE;

static Expr _SET = { .tag = ATOM, .atom = { .type = SYMBOL, .sval = "SET!" }, .protect = true, .mark = true };
Expr* SET = &_SET;

static Expr _IF = { .tag = ATOM, .atom = { .type = SYMBOL, .sval = "IF" }, .protect = true, .mark = true };
Expr* IF = &_IF;

bool scm_is_atom(const Expr* e) {
	assert(e);
	return e->tag == ATOM;
}
bool scm_is_pair(const Expr* e) {
	assert(e);
	return e != EMPTY_LIST && e->tag == PAIR;
}

bool scm_is_int(const Expr* e) {
	assert(e);
	return e->tag == ATOM && e->atom.type == INT;
}
bool scm_is_real(const Expr* e) {
	assert(e);
	return e->tag == ATOM && e->atom.type == REAL;
}
bool scm_is_bool(const Expr* e) {
	assert(e);
	return e->tag == ATOM && e->atom.type == BOOL;
}
bool scm_is_char(const Expr* e) {
	assert(e);
	return e->tag == ATOM && e->atom.type == CHAR;
}
bool scm_is_string(const Expr* e) {
	assert(e);
	return e->tag == ATOM && e->atom.type == STRING;
}
bool scm_is_symbol(const Expr* e) {
	assert(e);
	return e->tag == ATOM && e->atom.type == SYMBOL;
}
bool scm_is_error(const Expr* e) {
	assert(e);
	return e->tag == ATOM && e->atom.type == ERROR;
}
bool scm_is_ffunc(const Expr* e) {
	assert(e);
	return e->tag == ATOM && e->atom.type == FFUNC;
}
bool scm_is_true(const Expr* e) {
	assert(e);
	return e != FALSE;
}
bool scm_is_false(const Expr* e) {
	assert(e);
	return e == FALSE;
}
bool scm_is_list(const Expr* e) {
	assert(e);
	while(scm_is_pair(e)) {
		e = scm_cdr(e);
	}
	return e == EMPTY_LIST;
}

long long scm_ival(const Expr* e) {
	assert(e);
	assert(e->tag == ATOM && e->atom.type == INT);
	return e->atom.ival;
}
double scm_rval(const Expr* e) {
	assert(e);
	assert(e->tag == ATOM && e->atom.type == REAL);
	return e->atom.rval;
}
char scm_cval(const Expr* e) {
	assert(e);
	assert(e->tag == ATOM && e->atom.type == CHAR);
	return e->atom.cval;
}
char* scm_sval(const Expr* e) {
	assert(e);
	assert(e->tag == ATOM && (e->atom.type == STRING || e->atom.type == SYMBOL || e->atom.type == ERROR));
	return e->atom.sval;
}
bool scm_bval(const Expr* e) {
	assert(e);
	assert(e->tag == ATOM && e->atom.type == BOOL);
	return e->atom.bval;
}
ffunc scm_ffval(const Expr* e) {
	assert(e);
	assert(e->tag == ATOM && e->atom.type == FFUNC);
	return e->atom.ffptr;
}

Expr* scm_car(Expr* e) {
	assert(e);
	assert(e->tag == PAIR);
	return e->pair.car;
}
Expr* scm_cdr(Expr* e) {
	assert(e);
	assert(e->tag == PAIR);
	return e->pair.cdr;
}

Expr* scm_mk_int(int v) {
	Expr* toRet = NULL;

	if(toRet = scm_alloc()) {
		toRet->tag = ATOM;
		toRet->atom.type = INT;
		toRet->atom.ival = v;
		return toRet;
	}
	return toRet;
}

Expr* scm_mk_real(double v) {
	Expr* toRet = NULL;

	if(toRet = scm_alloc()) {
		toRet->tag = ATOM;
		toRet->atom.type = REAL;
		toRet->atom.rval = v;
	}
	return toRet;
}


Expr* scm_mk_char(char v) {
	Expr* toRet = NULL;

	if(toRet = scm_alloc()) {
		toRet->tag = ATOM;
		toRet->atom.type = CHAR;
		toRet->atom.cval = v;
		return toRet;
	}
	return NULL;
}

Expr* scm_mk_string(const char* v) {
	assert(v);

	Expr* toRet = NULL;
	
	if(toRet = scm_alloc()) {
		toRet->tag = ATOM;
		toRet->atom.type = STRING;
		toRet->atom.sval = strdup(v);
		return toRet;
	}
	return NULL;
}

Expr* scm_mk_symbol(const char* v) {
	assert(v);

	Expr* toRet = NULL;
	
	if(toRet = scm_mk_string(v)) {
		toRet->atom.type = SYMBOL;
		return toRet;
	}
	return NULL;
}

Expr* scm_mk_error(const char* v) {
	assert(v);

	Expr* toRet = NULL;
	
	if(toRet = scm_mk_string(v)) {
		toRet->atom.type = ERROR;
		return toRet;
	}
	return NULL;
}

Expr* scm_mk_pair(Expr* car, Expr* cdr) {
	assert(car);
	assert(cdr);

	Expr* toRet = NULL;
	
	if(toRet = scm_alloc()) {
		toRet->tag = PAIR;
		toRet->pair.car = car;
		toRet->pair.cdr = cdr;
		return toRet;
	}
	return NULL;
}
