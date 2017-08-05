/* This file contains definitions of functions operating on Exprs, and
 * definitions of the global variables in Scheme.h and SchemeSecret.h.
 */

#include "SchemeSecret.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

static const Expr _EMPTY_LIST = { .tag = ELIST, .pair = { NULL, NULL }, .protect = true, .mark = true };
Expr* EMPTY_LIST;

static const Expr _TRUE = { .tag = ATOM, .atom = { .type = BOOL, .bval = true }, .protect = true, .mark = true };
Expr* TRUE;

static const Expr _FALSE = { .tag = ATOM, .atom = { .type = BOOL, .bval = false }, .protect = true, .mark = true };
Expr* FALSE;

static const Expr _OOM = { .tag = ATOM, .atom = { .type = ERROR, .sval = "Out of memory" }, .protect = true, .mark = true };
Expr* OOM;

Expr* DEFINE = NULL;
Expr* SET = NULL;
Expr* IF = NULL;
Expr* LAMBDA = NULL;
Expr* QUOTE = NULL;
Expr* QUASIQUOTE = NULL;
Expr* UNQUOTE = NULL;
Expr* BEGIN = NULL;
Expr* COND = NULL;
Expr* ELSE = NULL;
Expr* AND = NULL;
Expr* OR = NULL;

// Keep a cache for characters since there are only 256 possible ones
#define mk_chr(x) { .tag = ATOM, .atom = { .type = CHAR, .cval = (char)(x) }, .protect = true, .mark = true }

static const Expr allChars[256] = {
	mk_chr(0x00), mk_chr(0x01), mk_chr(0x02), mk_chr(0x03), mk_chr(0x04), mk_chr(0x05), mk_chr(0x06), mk_chr(0x07),
	mk_chr(0x08), mk_chr(0x09), mk_chr(0x0A), mk_chr(0x0B), mk_chr(0x0C), mk_chr(0x0D), mk_chr(0x0E), mk_chr(0x0F),
	mk_chr(0x10), mk_chr(0x11), mk_chr(0x12), mk_chr(0x13), mk_chr(0x14), mk_chr(0x15), mk_chr(0x16), mk_chr(0x17),
	mk_chr(0x18), mk_chr(0x19), mk_chr(0x1A), mk_chr(0x1B), mk_chr(0x1C), mk_chr(0x1D), mk_chr(0x1E), mk_chr(0x1F),
	mk_chr(0x20), mk_chr(0x21), mk_chr(0x22), mk_chr(0x23), mk_chr(0x24), mk_chr(0x25), mk_chr(0x26), mk_chr(0x27),
	mk_chr(0x28), mk_chr(0x29), mk_chr(0x2A), mk_chr(0x2B), mk_chr(0x2C), mk_chr(0x2D), mk_chr(0x2E), mk_chr(0x2F),
	mk_chr(0x30), mk_chr(0x31), mk_chr(0x32), mk_chr(0x33), mk_chr(0x34), mk_chr(0x35), mk_chr(0x36), mk_chr(0x37),
	mk_chr(0x38), mk_chr(0x39), mk_chr(0x3A), mk_chr(0x3B), mk_chr(0x3C), mk_chr(0x3D), mk_chr(0x3E), mk_chr(0x3F),
	mk_chr(0x40), mk_chr(0x41), mk_chr(0x42), mk_chr(0x43), mk_chr(0x44), mk_chr(0x45), mk_chr(0x46), mk_chr(0x47),
	mk_chr(0x48), mk_chr(0x49), mk_chr(0x4A), mk_chr(0x4B), mk_chr(0x4C), mk_chr(0x4D), mk_chr(0x4E), mk_chr(0x4F),
	mk_chr(0x50), mk_chr(0x51), mk_chr(0x52), mk_chr(0x53), mk_chr(0x54), mk_chr(0x55), mk_chr(0x56), mk_chr(0x57),
	mk_chr(0x58), mk_chr(0x59), mk_chr(0x5A), mk_chr(0x5B), mk_chr(0x5C), mk_chr(0x5D), mk_chr(0x5E), mk_chr(0x5F),
	mk_chr(0x60), mk_chr(0x61), mk_chr(0x62), mk_chr(0x63), mk_chr(0x64), mk_chr(0x65), mk_chr(0x66), mk_chr(0x67),
	mk_chr(0x68), mk_chr(0x69), mk_chr(0x6A), mk_chr(0x6B), mk_chr(0x6C), mk_chr(0x6D), mk_chr(0x6E), mk_chr(0x6F),
	mk_chr(0x70), mk_chr(0x71), mk_chr(0x72), mk_chr(0x73), mk_chr(0x74), mk_chr(0x75), mk_chr(0x76), mk_chr(0x77),
	mk_chr(0x78), mk_chr(0x79), mk_chr(0x7A), mk_chr(0x7B), mk_chr(0x7C), mk_chr(0x7D), mk_chr(0x7E), mk_chr(0x7F),
	mk_chr(0x80), mk_chr(0x81), mk_chr(0x82), mk_chr(0x83), mk_chr(0x84), mk_chr(0x85), mk_chr(0x86), mk_chr(0x87),
	mk_chr(0x88), mk_chr(0x89), mk_chr(0x8A), mk_chr(0x8B), mk_chr(0x8C), mk_chr(0x8D), mk_chr(0x8E), mk_chr(0x8F),
	mk_chr(0x90), mk_chr(0x91), mk_chr(0x92), mk_chr(0x93), mk_chr(0x94), mk_chr(0x95), mk_chr(0x96), mk_chr(0x97),
	mk_chr(0x98), mk_chr(0x99), mk_chr(0x9A), mk_chr(0x9B), mk_chr(0x9C), mk_chr(0x9D), mk_chr(0x9E), mk_chr(0x9F),
	mk_chr(0xA0), mk_chr(0xA1), mk_chr(0xA2), mk_chr(0xA3), mk_chr(0xA4), mk_chr(0xA5), mk_chr(0xA6), mk_chr(0xA7),
	mk_chr(0xA8), mk_chr(0xA9), mk_chr(0xAA), mk_chr(0xAB), mk_chr(0xAC), mk_chr(0xAD), mk_chr(0xAE), mk_chr(0xAF),
	mk_chr(0xB0), mk_chr(0xB1), mk_chr(0xB2), mk_chr(0xB3), mk_chr(0xB4), mk_chr(0xB5), mk_chr(0xB6), mk_chr(0xB7),
	mk_chr(0xB8), mk_chr(0xB9), mk_chr(0xBA), mk_chr(0xBB), mk_chr(0xBC), mk_chr(0xBD), mk_chr(0xBE), mk_chr(0xBF),
	mk_chr(0xC0), mk_chr(0xC1), mk_chr(0xC2), mk_chr(0xC3), mk_chr(0xC4), mk_chr(0xC5), mk_chr(0xC6), mk_chr(0xC7),
	mk_chr(0xC8), mk_chr(0xC9), mk_chr(0xCA), mk_chr(0xCB), mk_chr(0xCC), mk_chr(0xCD), mk_chr(0xCE), mk_chr(0xCF),
	mk_chr(0xD0), mk_chr(0xD1), mk_chr(0xD2), mk_chr(0xD3), mk_chr(0xD4), mk_chr(0xD5), mk_chr(0xD6), mk_chr(0xD7),
	mk_chr(0xD8), mk_chr(0xD9), mk_chr(0xDA), mk_chr(0xDB), mk_chr(0xDC), mk_chr(0xDD), mk_chr(0xDE), mk_chr(0xDF),
	mk_chr(0xE0), mk_chr(0xE1), mk_chr(0xE2), mk_chr(0xE3), mk_chr(0xE4), mk_chr(0xE5), mk_chr(0xE6), mk_chr(0xE7),
	mk_chr(0xE8), mk_chr(0xE9), mk_chr(0xEA), mk_chr(0xEB), mk_chr(0xEC), mk_chr(0xED), mk_chr(0xEE), mk_chr(0xEF),
	mk_chr(0xF0), mk_chr(0xF1), mk_chr(0xF2), mk_chr(0xF3), mk_chr(0xF4), mk_chr(0xF5), mk_chr(0xF6), mk_chr(0xF7),
	mk_chr(0xF8), mk_chr(0xF9), mk_chr(0xFA), mk_chr(0xFB), mk_chr(0xFC), mk_chr(0xFD), mk_chr(0xFE), mk_chr(0xFF),
};

#undef mk_chr

// Keep a cache of small positive integers for the same reason
#define mk_int(x) { .tag = ATOM, .atom = { .type = INT, .cval = (x) }, .protect = true, .mark = true }

static const Expr someInts[32] = {
	mk_int(0x00), mk_int(0x01), mk_int(0x02), mk_int(0x03), mk_int(0x04), mk_int(0x05), mk_int(0x06), mk_int(0x07),
	mk_int(0x08), mk_int(0x09), mk_int(0x0A), mk_int(0x0B), mk_int(0x0C), mk_int(0x0D), mk_int(0x0E), mk_int(0x0F),
	mk_int(0x10), mk_int(0x11), mk_int(0x12), mk_int(0x13), mk_int(0x14), mk_int(0x15), mk_int(0x16), mk_int(0x17),
	mk_int(0x18), mk_int(0x19), mk_int(0x1A), mk_int(0x1B), mk_int(0x1C), mk_int(0x1D), mk_int(0x1E), mk_int(0x1F),

};

#undef mk_int

bool scm_is_atom(const Expr* e) {
	assert(e);
	return e->tag == ATOM;
}
bool scm_is_pair(const Expr* e) {
	assert(e);
	return e->tag == PAIR;
}
bool scm_is_closure(const Expr* e) {
	assert(e);
	return e->tag == CLOSURE;
}
bool scm_is_num(const Expr* e) {
	assert(e);
	return e->tag == ATOM && (e->atom.type == INT || e->atom.type == REAL);
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

Expr* scm_car(const Expr* e) {
	assert(e);
	assert(e->tag == PAIR || e->tag == CLOSURE);
	return e->pair.car;
}
Expr* scm_cdr(const Expr* e) {
	assert(e);
	assert(e->tag == PAIR || e->tag == CLOSURE);
	return e->pair.cdr;
}

Expr* scm_mk_int(long long v) {
	if(0 <= v && v < (long long)(sizeof(someInts)/sizeof(someInts[0]))) {
		return (Expr*) &someInts[v];
	}

	Expr* toRet = scm_alloc();

	if(toRet) {
		toRet->tag = ATOM;
		toRet->atom.type = INT;
		toRet->atom.ival = v;
	}
	return toRet;
}

Expr* scm_mk_real(double v) {
	Expr* toRet = scm_alloc();

	if(toRet) {
		toRet->tag = ATOM;
		toRet->atom.type = REAL;
		toRet->atom.rval = v;
	}
	return toRet;
}


Expr* scm_mk_char(char v) {
	return (Expr*) &allChars[(unsigned)v];
}

static char* strdup(const char* s) {
	size_t len = strlen(s);
	char* toRet = malloc(len + 1);
	strcpy(toRet, s);

	return toRet;
}

Expr* scm_mk_string(const char* v) {
	Expr* toRet = scm_alloc();

	if(toRet) {
		toRet->tag = ATOM;
		toRet->atom.type = STRING;
		toRet->atom.sval = strdup(v);
	}
	return toRet;
}

Expr* scm_mk_symbol(const char* v) {
	return scm_get_symbol(v);
}

Expr* scm_mk_error(const char* v) {
	Expr* toRet = scm_mk_string(v);

	if(toRet) toRet->atom.type = ERROR;
	else      toRet = OOM;

	return toRet;
}

Expr* scm_mk_pair(Expr* car, Expr* cdr) {
	assert(car);
	assert(cdr);

	Expr* toRet = scm_alloc();

	if(toRet) {
		toRet->tag = PAIR;
		toRet->pair.car = car;
		toRet->pair.cdr = cdr;
		return toRet;
	}
	return NULL;
}

Expr* scm_mk_list(Expr** l, size_t n) {
	Expr* toRet = EMPTY_LIST;
	scm_stack_push(&toRet);

	while(toRet && n != 0) {
		toRet = scm_mk_pair(l[n - 1], toRet);
		n--;
	}

	scm_stack_pop(&toRet);

	return toRet ? toRet : OOM;
}

Expr* scm_concat(Expr** l, size_t n) {
	if(n == 0) return EMPTY_LIST;

	Expr* toRet = l[n-1];
	scm_stack_push(&toRet);

	n--;
	while(toRet && n != 0) {
		toRet = scm_mk_pair(l[n-1], toRet);
		n--;
	}

	scm_stack_pop(&toRet);

	return toRet ? toRet : OOM;
}

Expr* scm_mk_closure(Expr* penv, Expr* args, Expr* body) {
	assert(penv);
	assert(args);
	assert(body);

	Expr* contents[3] = { penv, args, body };
	Expr* res = scm_mk_list(contents, 3);
	
	if(!scm_is_error(res)) {
		res->tag = CLOSURE;
	}

	return res;
}

int scm_list_len(Expr* l) {
	int soFar = 0;

	while(scm_is_pair(l)) {
		l = scm_cdr(l);
		soFar++;
	}

	return l == EMPTY_LIST ? soFar : -1;
}

Expr* scm_closure_env(Expr* c) {
	assert(c); assert(scm_is_closure(c));
	return scm_car(c);
}

Expr* scm_closure_args(Expr* c) {
	assert(c); assert(scm_is_closure(c));
	return scm_cadr(c);
}

Expr* scm_closure_body(Expr* c) {
	assert(c); assert(scm_is_closure(c));
	return scm_caddr(c);
}

void scm_init_expr() {
	DEFINE = scm_get_symbol("define");
	SET = scm_get_symbol("set!");
	IF = scm_get_symbol("if");
	LAMBDA = scm_get_symbol("lambda");
	QUOTE = scm_get_symbol("quote");
	QUASIQUOTE = scm_get_symbol("quasiquote");
	UNQUOTE = scm_get_symbol("unquote");
	BEGIN = scm_get_symbol("begin");
	COND = scm_get_symbol("cond");
	ELSE = scm_get_symbol("else");
	AND = scm_get_symbol("and");
	OR = scm_get_symbol("or");
	EMPTY_LIST = (Expr*) &_EMPTY_LIST;
	TRUE = (Expr*) &_TRUE;
	FALSE = (Expr*) &_FALSE;
	OOM = (Expr*) &_OOM;
}

void scm_reset_expr() {
	DEFINE = NULL;
	SET = NULL;
	IF = NULL;
	LAMBDA = NULL;
	QUOTE = NULL;
	QUASIQUOTE = NULL;
	UNQUOTE = NULL;
	BEGIN = NULL;
	COND = NULL;
	ELSE = NULL;
	AND = NULL;
	OR = NULL;
	EMPTY_LIST = NULL;
	TRUE = NULL;
	FALSE = NULL;
	OOM = NULL;
}
