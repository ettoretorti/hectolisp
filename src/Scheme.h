#pragma once

#include <stdbool.h>

typedef struct Expr {
	enum { ATOM, PAIR } tag;
	union {
		struct {
			enum { INT, CHAR, STRING, SYMBOL, BOOL } type;
			union {
				int ival;
				char* sval;
				char cval;
				bool bval;
			};
		} atom;

		struct {
			struct Expr* car;
			struct Expr* cdr;
		} pair;
	};
	bool protect;
	bool mark;
} Expr;


// SCECIAL FORMS
Expr* EMPTY_LIST;
Expr* TRUE;
Expr* FALSE;

Expr* DEFINE;
Expr* SET;
Expr* IF;


// EXPR PREDICATES
bool scm_is_atom(const Expr* e);
bool scm_is_pair(const Expr* e);

bool scm_is_int(const Expr* e);
bool scm_is_bool(const Expr* e);
bool scm_is_char(const Expr* e);
bool scm_is_string(const Expr* e);
bool scm_is_symbol(const Expr* e);


// EXPR ACCESSORS
int   scm_ival(const Expr* e);
char  scm_cval(const Expr* e);
char* scm_sval(const Expr* e);
bool  scm_bval(const Expr* e);
Expr* scm_car(Expr* e);
Expr* scm_cdr(Expr* e);


// EXPR_CONSTRUCTORS
Expr* scm_mk_int(int v);
Expr* scm_mk_char(char v);
Expr* scm_mk_string(const char* v);
Expr* scm_mk_symbol(const char* v);
Expr* scm_mk_pair(Expr* car, Expr* cdr);


// GENERAL
void scm_init();
void scm_reset();
void scm_gc();

Expr* scm_read(const char* in);
Expr* scm_eval(Expr* expr);
char* scm_print(Expr* expr);
