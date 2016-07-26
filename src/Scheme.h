#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Expr Expr;

typedef Expr *(*ffunc)(Expr*);

struct __attribute__((packed)) Expr {
	union {
		struct {
			enum { INT, REAL, CHAR, STRING, SYMBOL, BOOL, ERROR, FFUNC } type;
			union {
				long long ival;
				double rval;
				char* sval;
				char cval;
				bool bval;
				ffunc ffptr;
			};
		} atom;

		struct {
			struct Expr* car;
			struct Expr* cdr;
		} pair;
	};
	enum { ATOM, PAIR } tag : 1;
	bool protect : 1;
	bool mark : 1;
};


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
bool scm_is_error(const Expr* e);

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
Expr* scm_mk_error(const char* v);
Expr* scm_mk_pair(Expr* car, Expr* cdr);

// MEMORY MANAGEMENT
void scm_gc();
void scm_protect(Expr* e);
void scm_unprotect(Expr* e);

// GENERAL
void scm_init();
void scm_reset();

Expr* scm_read(const char* in);
Expr* scm_eval(Expr* expr);
char* scm_print(Expr* expr);



#ifdef __cplusplus
}
#endif
