#pragma once

#include <stdbool.h>
#include <stddef.h>

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
	enum { ATOM, PAIR, CLOSURE } tag : 2;
	bool protect : 1;
	bool mark : 1;
};


// GLOBALS
extern Expr* EMPTY_LIST;
extern Expr* TRUE;
extern Expr* FALSE;

// EXPR PREDICATES
bool scm_is_atom(const Expr* e);
bool scm_is_pair(const Expr* e);
bool scm_is_closure(const Expr* e);

bool scm_is_num(const Expr* e);
bool scm_is_int(const Expr* e);
bool scm_is_real(const Expr* e);
bool scm_is_bool(const Expr* e);
bool scm_is_char(const Expr* e);
bool scm_is_string(const Expr* e);
bool scm_is_symbol(const Expr* e);
bool scm_is_error(const Expr* e);
bool scm_is_ffunc(const Expr* e);

bool scm_is_true(const Expr* e);
bool scm_is_false(const Expr* e);
bool scm_is_list(const Expr* e);


// EXPR ACCESSORS
long long scm_ival(const Expr* e);
double    scm_rval(const Expr* e);
char      scm_cval(const Expr* e);
char*     scm_sval(const Expr* e);
bool      scm_bval(const Expr* e);
ffunc     scm_ffval(const Expr* e);
Expr*     scm_car(const Expr* e);
Expr*     scm_cdr(const Expr* e);
#define scm_caar(e)   scm_car(scm_car(e))
#define scm_cadr(e)   scm_car(scm_cdr(e))
#define scm_cdar(e)   scm_cdr(scm_car(e))
#define scm_cddr(e)   scm_cdr(scm_cdr(e))
#define scm_caaar(e)  scm_car(scm_car(scm_car(e)))
#define scm_caadr(e)  scm_car(scm_car(scm_cdr(e)))
#define scm_cadar(e)  scm_car(scm_cdr(scm_car(e)))
#define scm_caddr(e)  scm_car(scm_cdr(scm_cdr(e)))
#define scm_cdaar(e)  scm_cdr(scm_car(scm_car(e)))
#define scm_cdadr(e)  scm_cdr(scm_car(scm_cdr(e)))
#define scm_cddar(e)  scm_cdr(scm_cdr(scm_car(e)))
#define scm_cdddr(e)  scm_cdr(scm_cdr(scm_cdr(e)))

// EXPR_CONSTRUCTORS
Expr* scm_mk_int(int v);
Expr* scm_mk_real(double v);
Expr* scm_mk_char(char v);
Expr* scm_mk_string(const char* v);
Expr* scm_mk_symbol(const char* v);
Expr* scm_mk_error(const char* v);
Expr* scm_mk_pair(Expr* car, Expr* cdr);

// ADVANCED CONSTRUCTORS
Expr* scm_mk_list(Expr** l, size_t n);

// MISCELLANEOUS
int scm_list_len(Expr* l); // returns -1 when not given a proper list

// MEMORY MANAGEMENT
void scm_gc();
void scm_protect(Expr* e);
void scm_unprotect(Expr* e);

void scm_stack_push(Expr** e);
void scm_stack_pop(Expr** e);

// GENERAL
void scm_init();
void scm_reset();

unsigned scm_gc_runs();
unsigned scm_gc_free_objects();

Expr* scm_read(const char* in);
Expr* scm_read_inc(const char* in, char** rem);
Expr* scm_eval(Expr* expr);
char* scm_print(Expr* expr);



#ifdef __cplusplus
}
#endif
