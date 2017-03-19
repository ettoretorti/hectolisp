/* This file represents environments. Each "frame" is simply a list of:
 *   (parent-env names values)
 * where
 *   parent-env is the parent environment if it exists, false otherwise,
 *   names is the list of names of the bound variables in this environment,
 *   values contains the values bound to the names of this environemnt
 */

#include "SchemeSecret.h"

#include <assert.h>
#include <string.h>

Expr* BASE_ENV = NULL;
Expr* CURRENT_ENV = NULL;

static int idxOf(Expr* sym, Expr* list) {
	assert(sym); assert(list);
	
	int cur = 0;
	while(scm_is_pair(list)) {
		if(scm_car(list) == sym) {
			return cur;
		}

		list = scm_cdr(list);
		cur++;
	}

	return -1;
}

static Expr* get(Expr* list, int idx) {
	assert(list); assert(idx >= 0);

	while(scm_is_pair(list) && idx > 0) {
		list = scm_cdr(list);
		idx--;
	}

	assert(idx == 0);

	return scm_car(list);
}

static Expr* replace(int idx, Expr* list, Expr* val) {
	assert(idx >= 0); assert(list); assert(val);

	while(scm_is_pair(list) && idx > 0) {
		list = scm_cdr(list);
		idx--;
	}

	assert(idx == 0);

	Expr* toRet = scm_car(list);
	list->pair.car = val;
	return toRet;
}

Expr* scm_env_lookup(Expr* env, Expr* sym) {
	assert(env); assert(sym); assert(scm_is_symbol(sym));

	while(env != FALSE) {
		Expr* names = scm_cadr(env);
		int idx = idxOf(sym, names);

		if(idx != -1) {
			return get(scm_caddr(env), idx);
		}

		env = scm_car(env);
	}

	char buf[256];
	buf[0] = '\0';
	strcat(buf, "can't get unbound symbol: ");
	strcat(buf, scm_sval(sym));

	return scm_mk_error(buf);
}

Expr* scm_env_define(Expr* env, Expr* sym, Expr* val) {
	assert(env); assert(sym); assert(val);

	int idx = idxOf(sym, scm_cadr(env));

	if(idx == -1) {
		scm_cdr(env)->pair.car = scm_mk_pair(sym, scm_cadr(env));
		scm_cddr(env)->pair.car = scm_mk_pair(val, scm_caddr(env));

		return val;
	} else {
		//TODO not sure overriding anyway is the best option...
		return replace(idx, scm_caddr(env), val);
	}
}

Expr* scm_env_set(Expr* env, Expr* sym, Expr* val) {
	assert(env); assert(sym); assert(val);

	while(env != FALSE) {
		int idx = idxOf(sym, scm_cadr(env));
		if(idx != -1) {
			return replace(idx, scm_caddr(env), val);
		}

		env = scm_car(env);
	}

	char buf[256];
	buf[0] = '\0';
	strcat(buf, "Can't set unbound symbol: ");
	strcat(buf, scm_sval(sym));

	return scm_mk_error(buf);
}

Expr* scm_mk_env(Expr* parent, Expr* names, Expr* vals) {
	assert(parent); assert(names); assert(vals);

	Expr* ll[3] = { parent, names, vals };
	return scm_mk_list(ll, 3);
}


void scm_env_push(Expr* names, Expr* vals) {
	assert(CURRENT_ENV);

	Expr* new = scm_mk_env(CURRENT_ENV, names, vals);
	CURRENT_ENV = new;
}

void scm_env_pop() {
	assert(CURRENT_ENV != BASE_ENV);

	CURRENT_ENV = scm_car(CURRENT_ENV);
}

void scm_init_env() {
	BASE_ENV = scm_mk_env(FALSE, EMPTY_LIST, EMPTY_LIST);
	CURRENT_ENV = BASE_ENV;
}

void scm_reset_env() {
	BASE_ENV = NULL;
	CURRENT_ENV = NULL;
}
