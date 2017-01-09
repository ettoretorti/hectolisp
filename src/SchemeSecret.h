#pragma once

#include "Scheme.h"

#ifdef __cplusplus
extern "C" {
#endif

//Standard library
extern const unsigned char scm_stdlib[];
extern const unsigned scm_stdlib_len;

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

Expr* scm_mk_env(Expr* parent, Expr* names, Expr* vals);

void scm_env_push(Expr* names, Expr* vals);
void scm_env_pop();

// Returns an scm error on failure
Expr* scm_env_lookup(Expr* env, Expr* sym);

// Returns val on success, an scm error on failure
Expr* scm_env_define(Expr* env, Expr* sym, Expr* val);

// Returns val on success, an scm_error on failure
Expr* scm_env_set(Expr* env, Expr* sym, Expr* val);

//Error Messages
extern Expr* OOM;

void scm_init_expr();
void scm_reset_expr();

//Symbols
Expr* scm_get_symbol(const char* s);
void scm_reset_symbol_set();

//Functions
void scm_init_func();

//Closures
Expr* scm_mk_closure(Expr* penv, Expr* args, Expr* body);
Expr* scm_closure_env(Expr* c);
Expr* scm_closure_args(Expr* c);
Expr* scm_closure_body(Expr* c);

#ifdef __cplusplus
}
#endif
