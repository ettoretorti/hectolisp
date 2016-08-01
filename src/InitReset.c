#include "SchemeSecret.h"

void scm_init() {
	scm_init_mem();
	scm_init_expr();
}

void scm_reset() {
	scm_gc();
	scm_reset_expr();
	scm_reset_symbol_set();
}
