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

void scm_init_mem();
Expr* scm_alloc();


#ifdef __cplusplus
}
#endif
