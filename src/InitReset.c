/* This file defines scm_init() and scm_reset() by aggregating all the
 * init() and reset() functions defined in SchemeSecret.h together.
 */

#include "SchemeSecret.h"

#include <assert.h>

void scm_init() {
	scm_init_mem();
	scm_init_expr();
	scm_init_env();
	scm_init_func();
	scm_init_stdlib();
}

void scm_reset() {
	scm_reset_expr();
	scm_reset_env();
	scm_gc();
	scm_reset_symbol_set();
}

void scm_init_stdlib() {
	char* src = (char*) scm_stdlib;

	while(*src != '\0') {
		Expr* red = scm_read_inc(src, &src);
		Expr* res = scm_eval(red);
		assert(res); assert(!scm_is_error(res)); (void)res;
	}
}
