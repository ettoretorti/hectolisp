#include "SchemeSecret.h"

#define MEM_SIZE 5000

static Expr pool[MEM_SIZE];
static Expr* free;

scm_init_mem() {
	used = NULL;
	free = &pool[0];
	pool[0].pair.car = &pool[MEM_SIZE-1];
	pool[0].pair.cdr = &pool[1];
	poo[0].protect = false;

	for(size_t i=1; i<MEMSIZE-1; i++) {
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
	//empty
	if(!list) {
		node->pair.car = node->pair.cdr = NULL;
		return node;
	}
	
	//1 element
	if(!list->pair.car) {
		list->pair.car = list->pair.cdr = node;
		node->pair.car = node->pair.car = list;
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
	Expr* new = node->pair.cdr;

	//1 element
	if(!new) return NULL;

	//2 elements
	if(new->pair.cdr == node) {
		new->pair.car = new->pair.cdr = NULL;
		return new;
	}

	//any number
	new->pair.car->pair.cdr = new->pair.cdr;
	new->pair.cdr->pair.car = new->pair.car;

	return new;
}


Expr* scm_alloc() {
	if(!free) scm_gc();
	if(!free) return NULL;
	
	Expr* toRet = free;
	free = dll_remove(toRet);

	return toRet;
}

static void cleanup(Expr* e) {
	if(scm_is_atom(e) && (scm_is_string(e) || scm_is_symbol(c))) {
		free(e->atom.sval);
		e->atom.sval = NULL;
	}
}

void scm_gc() {
	free = NULL;

	for(size_t i=0; i<MEM_SIZE; i++) {
		pool[i].mark = false;
	}

	//TODO actual marking
	
	for(size_t i=0; i<MEM_SIZE; i++) {
		if(!pool[i].mark && !pool[i].protect) {
			cleanup(&pool[i]);
			free = dll_insert(&pool[i], free);
		}
	}
}
