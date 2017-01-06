/* This file handles all of the memory management and garbage collection. The
 * basic ideas behind it are:
 *   - A fixed-size pool of Exprs is allocated statically
 *   - A doubly-linked freelist is created out of them using the pair cells
 *   - Allocating an Expr involves extracting the head of the freelist
 *   - Freeing an Expr involves inserting it back into the freelist
 *
 * Garbage collection is done by resetting the mark bits of all the Exprs in the
 * pool, followed by doing a recursive marking of Exprs in use starting from
 * known entry points (the scheme environment) and Exprs that have their
 * protected bits set. Once this is done, all unmarked Exprs are linked
 * together to form a new freelist.
 *
 * TODO:
 *   - Use the Schorr-Deutch-Waite link-inversion algorithm for marking
 *   - Allow multiple dynamic pools
 *   - Switch to an incremental gc algorithm
 */

#include "SchemeSecret.h"
#include <stddef.h>
#include <assert.h>

#define MEM_SIZE 5000
#define STACK_SIZE 1024

//TODO: remove
extern void free(void* ptr);

static Expr pool[MEM_SIZE];
static Expr* freeList = NULL;

static Expr** protStack[STACK_SIZE];
static size_t protStackSize = 0;

void scm_init_mem() {
	freeList = &pool[0];
	pool[0].pair.car = &pool[MEM_SIZE-1];
	pool[0].pair.cdr = &pool[1];
	pool[0].protect = false;

	for(size_t i=1; i<MEM_SIZE-1; i++) {
		pool[i].pair.car = &pool[i-1];
		pool[i].pair.cdr = &pool[i+1];
		pool[i].protect = false;
	}

	//circular doubly linked list
	pool[MEM_SIZE-1].pair.car = &pool[MEM_SIZE-2];
	pool[MEM_SIZE-1].pair.cdr = &pool[0];
	pool[MEM_SIZE-1].protect = false;
}

static Expr* dll_insert(Expr* node, Expr* list) {
	assert(node);

	//empty
	if(!list) {
		node->pair.car = node->pair.cdr = NULL;
		return node;
	}

	//1 element
	if(!list->pair.car) {
		assert(!list->pair.cdr);
		list->pair.car = list->pair.cdr = node;
		node->pair.car = node->pair.cdr = list;
		return node;
	}

	//any number
	node->pair.car = list->pair.car;
	node->pair.cdr = list;

	node->pair.car->pair.cdr = node;
	node->pair.cdr->pair.car = node;

	return node;
}

static Expr* dll_remove(Expr* node) {
	assert(node);

	Expr* new = node->pair.cdr;

	//1 element
	if(!new) return NULL;

	//2 elements
	if(new->pair.cdr == node) {
		new->pair.car = new->pair.cdr = NULL;
		return new;
	}

	//any number
	new->pair.car = node->pair.car;
	node->pair.car->pair.cdr = new;

	return new;
}

Expr* scm_alloc() {
	if(!freeList) scm_gc();
	if(!freeList) return NULL;

	Expr* toRet = freeList;
	freeList = dll_remove(toRet);

	return toRet;
}

static void cleanup(Expr* e) {
	assert(e);

	if(scm_is_atom(e) && (scm_is_string(e) || scm_is_symbol(e) || scm_is_error(e))) {
		free(e->atom.sval);
		e->atom.sval = NULL;
	}
}

static void mark(Expr* e) {
	assert(e);

	if(e->mark) return;

	e->mark = true;
	if(scm_is_pair(e) || scm_is_closure(e)) {
		mark(scm_car(e));
		mark(scm_cdr(e));
	}
}

void scm_protect(Expr* e) {
	assert(e);
	e->protect = true;
}

void scm_unprotect(Expr* e) {
	assert(e);
	e->protect = false;
}

void scm_stack_push(Expr** e) {
	assert(e);
	assert(protStackSize < STACK_SIZE);

	protStack[protStackSize++] = e;
}

void scm_stack_pop(Expr** e) {
	assert(e);
	assert(protStackSize > 0);
	assert(e == protStack[protStackSize - 1]);

	protStackSize--;
}

void scm_gc() {
	freeList = NULL;

	for(size_t i = 0; i < MEM_SIZE; i++) {
		pool[i].mark = false;
	}

	for(size_t i = 0; i < MEM_SIZE; i++) {
		if(pool[i].protect) {
			mark(&pool[i]);
		}
	}

	for(size_t i = 0; i < protStackSize; i++) {
		mark(*protStack[i]);
	}

	//TODO actual marking
	if(BASE_ENV)    mark(BASE_ENV);
	if(CURRENT_ENV) mark(CURRENT_ENV);

	for(size_t i=0; i<MEM_SIZE; i++) {
		if(!pool[i].mark && !pool[i].protect) {
			cleanup(&pool[i]);
			freeList = dll_insert(&pool[i], freeList);
		}
	}
}
