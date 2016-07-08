#pragma once

#include "Scheme.h"

#ifdef __cplusplus
extern "C" {
#endif

void scm_init_mem();
Expr* scm_alloc();


#ifdef __cplusplus
}
#endif
