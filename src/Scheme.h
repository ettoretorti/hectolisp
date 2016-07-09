#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Expr {
	enum { ATOM, PAIR } tag;
	union {
		struct {
			enum { INT, REAL, CHAR, STRING, SYMBOL, BOOL } type;
			union {
				long long ival;
				double rval;
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
extern Expr* EMPTY_LIST;
extern Expr* TRUE;
extern Expr* FALSE;

extern Expr* DEFINE;
extern Expr* SET;
extern Expr* IF;


// EXPR PREDICATES
bool scm_is_atom(const Expr* e);
bool scm_is_pair(const Expr* e);

bool scm_is_int(const Expr* e);
bool scm_is_real(const Expr* e);
bool scm_is_bool(const Expr* e);
bool scm_is_char(const Expr* e);
bool scm_is_string(const Expr* e);
bool scm_is_symbol(const Expr* e);

bool scm_is_true(const Expr* e);


// EXPR ACCESSORS
long long scm_ival(const Expr* e);
double    scm_rval(const Expr* e);
char      scm_cval(const Expr* e);
char*     scm_sval(const Expr* e);
bool      scm_bval(const Expr* e);
Expr*     scm_car(Expr* e);
Expr*     scm_cdr(Expr* e);


// EXPR_CONSTRUCTORS
Expr* scm_mk_int(int v);
Expr* scm_mk_real(double v);
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



#ifdef __cplusplus
}
#endif
