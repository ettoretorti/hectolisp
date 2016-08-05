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
extern Expr* AND;
extern Expr* OR;

//Memory
void scm_init_mem();
Expr* scm_alloc();

//Environments
extern Expr* BASE_ENV;
extern Expr* CURRENT_ENV;

void scm_init_env();
void scm_reset_env();

void scm_env_push(Expr* names, Expr* vals);
void scm_env_pop();

Expr* scm_env_lookup(Expr* env, Expr* sym);
Expr* scm_env_define(Expr* env, Expr* sym, Expr* val);
Expr* scm_env_set(Expr* env, Expr* sym, Expr* val);

//Error Messages
extern Expr* OOM;

void scm_init_expr();
void scm_reset_expr();

//Symbols
Expr* scm_get_symbol(const char* s);
void scm_reset_symbol_set();

#ifdef __cplusplus
}
#endif
