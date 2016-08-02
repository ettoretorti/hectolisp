/* This file handles the memory allocation of symbols. Symbols are handled
 * separately to ensure that any two symbols with the same "name" are
 * represented by the same unique symbol Expr.
 *
 * Uniqueness is guaranteed by allocating all symbols in a set backed by an
 * AVL tree.
 */

#include "SchemeSecret.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct __attribute__((packed)) AVL {
	Expr* v;
	struct AVL* l;
	struct AVL* r;
	short h;
} AVL;

static AVL* symbols = NULL;

static inline short h(AVL* avl) {
	if(!avl) return 0;
	else     return avl->h;
}

static inline AVL* l(AVL* avl) {
	if(!avl) return NULL;
	else     return avl->l;
}

static inline AVL* r(AVL* avl) {
	if(!avl) return NULL;
	else     return avl->r;
}

//     avl                 x
//   a      x   -->   avl    c
//        b   c     a     b
static AVL* avl_rl(AVL* avl) {
	assert(avl); assert(avl->r);

	AVL* x = avl->r;
	AVL* b = l(x);

	avl->r = b;
	x->l = avl;

	return x;
}

//        avl         x
//     x     c --> a     avl
//   a   b             b     c
static AVL* avl_rr(AVL* avl) {
	assert(avl); assert(avl->l);

	AVL* x = avl->l;
	AVL* b = r(x);

	avl->l = b;
	x->r = avl;

	return x;
}

static Expr* avl_find(AVL* avl, const char* key) {
//just a tail call disguised as a goto
start:
	if(!avl) return FALSE;

	const char* val = scm_sval(avl->v);
	int cmp = strcmp(key, val);

	if(cmp == 0) {
		return avl->v;
	} else if(cmp < 0) {
		avl = avl->l;
		goto start;
	} else {
		avl = avl->r;
		goto start;
	}
}

static AVL* avl_insert(AVL* avl, const char* key, Expr** res) {
	if(!avl) {
		//bypass the scheme GC since interned symbols are forever anyway
		AVL* new = (AVL*) malloc(sizeof(AVL));
		if(!new) {
			*res = NULL;
			return NULL;
		}
		new->l = new->r = NULL;
		new->h = 1;
		new->v = (Expr*) malloc(sizeof(Expr));
		if(!new->v) {
			free(new);
			*res = NULL;
			return NULL;
		}
		new->v->mark = new->v->protect = true;
		new->v->tag = ATOM;
		new->v->atom.type = SYMBOL;
		new->v->atom.sval = strdup(key);
		*res = new->v;

		return new;
	}

	const char* val = scm_sval(avl->v);
	int cmp = strcmp(key, val);

	if(cmp == 0) {
		*res = avl->v;
	} else if(cmp < 0) {
		avl->l = avl_insert(avl->l, key, res);

		if(avl->l->h > h(avl->r) + 1) {
			if(h(avl->l->r) > h(avl->l->l)) {
				avl->l = avl_rl(avl->l);
			}

			avl = avl_rr(avl);
		}
	} else if(cmp > 0) {
		avl->r = avl_insert(avl->r, key, res);

		if(avl->r->h > h(avl->l) + 1) {
			if(h(avl->r->l) > h(avl->r->r)) {
				avl->r = avl_rr(avl->r);
			}
			avl = avl_rl(avl);
		}
	}

	short lh = h(avl->l);
	short rh = h(avl->r);

	avl->h = (lh > rh ? lh : rh) + 1;

	return avl;
}

static void avl_free(AVL* avl) {
	if(avl) {
		free(scm_sval(avl->v));
		free(avl->v);

		avl_free(avl->l);
		avl_free(avl->r);
		free(avl);
	}
}

Expr* scm_get_symbol(const char* s) {
	Expr* res = NULL;
	symbols = avl_insert(symbols, s, &res);

	return res;
}

void scm_reset_symbol_set() {
	avl_free(symbols);
	symbols = NULL;
}
