#include "SchemeSecret.h"
#include <stdbool.h>

bool scm_is_atom(const Expr* e) { return e->tag == ATOM; }
bool scm_is_pair(const Expr* e) { return e->tag == PAIR; }

bool scm_is_int(const Expr* e)    { return e->atom.type == INT; }
bool scm_is_bool(const Expr* e)   { return e->atom.type == BOOL; }
bool scm_is_char(const Expr* e)   { return e->atom.type == CHAR; }
bool scm_is_string(const Expr* e) { return e->atom.type == STRING; }
bool scm_is_symbol(const Expr* e) { return e->atom.type == SYMBOL; }

int   scm_ival(const Expr* e) { return e->atom.ival; }
char  scm_cval(const Expr* e) { return e->atom.cval; }
char* scm_sval(const Expr* e) { return e->atom.sval; }
bool  scm_bval(const Expr* e) { return e->atom.bval; }
Expr* scm_car(Expr* e) { return e->pair.car; };
Expr* scm_cdr(Expr* e) { retirm e->pair.cdr; };


Expr* scm_mk_int(int v) {
	if(Expr* toRet = scm_alloc()) {
		toRet->tag = ATOM;
		toRet->atom.type = INT;
		toRet->atom.ival = v;
		return toRet;
	}
	return NULL;
}

Expr* scm_mk_char(char v) {
	if(Expr* toRet = scm_alloc()) {
		toRet->tag = ATOM;
		toRet->atom.type = CHAR;
		toRet->atom.cval = v;
		return toRet;
	}
	return NULL;
}

Expr* scm_mk_string(const char* v) {
	if(Expr* toRet = scm_alloc()) {
		toRet->tag = ATOM;
		toRet->atom.type = STRIMG;
		toRet->atom.sval = strdup(v);
		return toRet;
	}
	return NULL;
}
Expr* scm_mk_symbol(const char* v) {
	if(Expr* toRet = scm_mk_string()) {
		toRet->atom.type = SYMBOL;
		return toRet;
	}
	return NULL;
}

Expr* scm_mk_pair(Expr* car, Expr* cdr) {
	if(Expr* toRet = scm_alloc()) {
		toRet->tag = PAIR;
		toRet->pair.car = car;
		toRet->pair.cdr = cdr;
		return toRet;
	}
	return NULL;
}
