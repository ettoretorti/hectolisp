/* This file defines scm_init() and scm_reset() by aggregating all the
 * init() and reset() functions defined in SchemeSecret.h together.
 */

#include "SchemeSecret.h"

void scm_init() {
	scm_init_mem();
	scm_init_expr();
	scm_init_env();
}

void scm_reset() {
	scm_reset_expr();
	scm_reset_env();
	scm_gc();
	scm_reset_symbol_set();
}
