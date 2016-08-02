#pragma once

#include "Scheme.h"

#ifdef __cplusplus
extern "C" {
#endif

//Special syntax
extern Expr* DEFINE;
extern Expr* SET;
extern Expr* IF;
extern Expr* LAMBDA;
extern Expr* QUOTE;
extern Expr* BEGIN;
extern Expr* COND;
extern Expr* ELSE;

//Error Messages
extern Expr* OOM;

void scm_init_mem();
void scm_init_expr();
void scm_reset_expr();
Expr* scm_alloc();

Expr* scm_get_symbol(const char* s);
void scm_reset_symbol_set();

#ifdef __cplusplus
}
#endif
