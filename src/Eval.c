#include "Scheme.h"
#include "SchemeSecret.h"

#include <assert.h>

static bool is_tpair(Expr* e, Expr* tag) {
	assert(e); assert(tag); assert(scm_is_symbol(tag));

	return scm_is_pair(e) && scm_is_symbol(scm_car(e)) && scm_car(e) == tag;
}

static inline bool is_last(Expr* l) {
	assert(scm_is_pair(l));
	
	return !scm_is_pair(scm_cdr(l));
}

static Expr* save_eval(Expr* es) {
	Expr* curEnv = CURRENT_ENV;
	scm_stack_push(&curEnv);

	Expr* res = scm_eval(es);

	CURRENT_ENV = curEnv;
	scm_stack_pop(&curEnv);

	return res;
}

// short circuits errors up the call stack
#define error_circuit(expr) \
	{ Expr* TmP = expr; \
	  if(scm_is_error(TmP)) return TmP; }

static Expr* save_eval_all(Expr* es) {
	assert(es);

	if(es == EMPTY_LIST) {
		return EMPTY_LIST;
	}

	Expr* curEnv = CURRENT_ENV;
	scm_stack_push(&curEnv);

	Expr* head = scm_mk_pair(EMPTY_LIST, EMPTY_LIST);
	scm_stack_push(&head);
	Expr* cur = head;

	while(scm_is_pair(es)) {
		cur->pair.car = scm_eval(scm_car(es));
		error_circuit(scm_car(cur));

		if(scm_is_pair(scm_cdr(es))) {
			cur->pair.cdr = scm_mk_pair(EMPTY_LIST, EMPTY_LIST);
			if(!cur->pair.cdr) return OOM;
			cur = scm_cdr(cur);
		}

		es = scm_cdr(es);
		
		CURRENT_ENV = curEnv;
	}

	if(es != EMPTY_LIST) {
		return scm_mk_error("arguments aren't a proper list");
	}
	
	scm_stack_pop(&head);
	scm_stack_pop(&curEnv);

	return head;
}

#undef error_circuit


// needs modification to deal with the tailcall cleanup
// TODO STILL BROKEN IN CASES OTHER THINGS ARE STACKED FIRST
#define error_circuit(expr) \
	{ Expr* TmP = expr; \
	  if(scm_is_error(TmP)) { scm_stack_pop(&e); return TmP; } }

Expr* scm_eval(Expr* e) {
	scm_stack_push(&e);
//just a tail call
begin:
	assert(e);

	if(scm_is_pair(e)) {
		if(is_tpair(e, QUOTE)) {
			scm_stack_pop(&e);
			return scm_cadr(e);
		} else if(is_tpair(e, IF)) {
			//(if predicate consequent alternative)
			//ensure list length is correct, error out otherwise
			e = scm_cdr(e);
			if(!scm_is_pair(e)) goto error;
			Expr* predicate = scm_car(e);

			e = scm_cdr(e);
			if(!scm_is_pair(e)) goto error;
			Expr* consequent = scm_car(e);

			e = scm_cdr(e);
			if(!scm_is_pair(e)) goto error;
			Expr* alternative = scm_car(e);

			if(scm_cdr(e) != EMPTY_LIST) goto error;

			Expr* predEvaled = save_eval(predicate);
			error_circuit(predEvaled);

			if(scm_is_true(predEvaled)) {
				e = consequent;
			} else {
				e = alternative;
			}
			goto begin;

			error:
				scm_stack_pop(&e);
				return scm_mk_error("Incorrect number of args to if (expected 3)");
		} else if(is_tpair(e, BEGIN)) {
			e = scm_cdr(e);
			
			while(scm_is_pair(e) && scm_cdr(e) != EMPTY_LIST) {
				Expr* res = save_eval(scm_car(e));
				error_circuit(res);

				e = scm_cdr(e);
			}
			
			if(!scm_is_pair(e) || scm_cdr(e) != EMPTY_LIST) {
				scm_stack_pop(&e);
				return scm_mk_error("sequence of expressions to evaluate isn't a proper list");
			}

			e = scm_car(e);
			goto begin;
		} else if(is_tpair(e, COND)) {
			e = scm_cdr(e);

			while(scm_is_pair(e)) {
				Expr* clause = scm_car(e);

				if(!scm_is_pair(clause) || !scm_is_pair(scm_cdr(clause))) {
					scm_stack_pop(&e);
					return scm_mk_error("Malformed cond clause");
				}

				Expr* predicate = scm_car(clause);

				bool go = predicate == ELSE;
				if(!go) {
					Expr* predEvaled = save_eval(predicate);
					error_circuit(predEvaled);

					go = scm_is_true(predEvaled);
				}

				if(go) {
					e = scm_mk_pair(BEGIN, scm_cdr(clause));
					if(!e) {
						scm_stack_pop(&e);
						return OOM;
					}
					goto begin;
				}

				e = scm_cdr(e);
			}

			if(e != EMPTY_LIST) {
				scm_stack_pop(&e);
				return scm_mk_error("sequence of clauses in cond isn't a proper list");
			}

			//no matching clause in cond is unspecified
			scm_stack_pop(&e);
			return EMPTY_LIST;
		} else if(is_tpair(e, DEFINE)) {
			e = scm_cdr(e);

			if(!scm_is_pair(e) || !scm_is_symbol(scm_car(e)) || !scm_is_pair(scm_cdr(e)) || scm_cddr(e) != EMPTY_LIST) {
				scm_stack_pop(&e);
				return scm_mk_error("Malformed define");
			}

			Expr* name = scm_car(e);
			Expr* val = save_eval(scm_cadr(e));
			error_circuit(val);

			scm_stack_push(&val);
			Expr* res = scm_env_define(CURRENT_ENV, name, val);
			scm_stack_pop(&val);

			scm_stack_pop(&e);
			return res;
		} else if(is_tpair(e, SET)) {
			e = scm_cdr(e);
			
			if(!scm_is_pair(e) || !scm_is_symbol(scm_car(e)) || !scm_is_pair(scm_cdr(e)) || scm_cddr(e) != EMPTY_LIST) {
				scm_stack_pop(&e);
				return scm_mk_error("Malformed set!");
			}

			Expr* name = scm_car(e);
			Expr* val = save_eval(scm_cadr(e));
			error_circuit(val);
			
			scm_stack_push(&val);
			Expr* res = scm_env_set(CURRENT_ENV, name, val);
			scm_stack_pop(&val);

			scm_stack_pop(&e);
			return res;
		} else if(is_tpair(e, AND)) {
			e = scm_cdr(e);
			if(e == EMPTY_LIST) {
				scm_stack_pop(&e);
				return TRUE;
			}
			
			while(!is_last(e)) {
				Expr* v = save_eval(scm_car(e));
				error_circuit(v);

				if(scm_is_false(v)) {
					scm_stack_pop(&e);
					return FALSE;
				}

				e = scm_cdr(e);
			}

			if(!scm_is_pair(e) || scm_cdr(e) != EMPTY_LIST) {
				scm_stack_pop(&e);
				return scm_mk_error("arguments to and aren't a proper list");
			}
			
			e = scm_car(e);
			goto begin;
		} else if(is_tpair(e, OR)) {
			e = scm_cdr(e);
			if(e == EMPTY_LIST) {
				scm_stack_pop(&e);
				return FALSE;
			}
			
			while(!is_last(e)) {
				Expr* v = save_eval(scm_car(e));
				error_circuit(v);

				if(scm_is_true(v)) {
					scm_stack_pop(&e);
					return v;
				}

				e = scm_cdr(e);
			}

			if(!scm_is_pair(e) || scm_cdr(e) != EMPTY_LIST) {
				scm_stack_pop(&e);
				return scm_mk_error("arguments to or aren't a proper list");
			}
			
			e = scm_car(e);
			goto begin;
		} else if(is_tpair(e, LAMBDA)) {
			// (lambda (the arg list) body)
			e = scm_cdr(e);
			
			if(!scm_is_pair(e)) {
				scm_stack_pop(&e);
				return scm_mk_error("missing argument list to lambda");
			}

			Expr* args = scm_car(e);
			if(!scm_is_pair(args) && args != EMPTY_LIST) {
				scm_stack_pop(&e);
				return scm_mk_error("lambda arguments aren't in a list");
			}

			Expr* body = scm_cdr(e);
			if(!scm_is_pair(body)) {
				scm_stack_pop(&e);
				return scm_mk_error("lambda body is not a list");
			}

			scm_stack_pop(&e);
			return scm_mk_closure(CURRENT_ENV, args, body);
		} else {
			//TODO GC safety
			Expr* func = save_eval(scm_car(e));
			error_circuit(func);
			scm_stack_push(&func);

			Expr* args = save_eval_all(scm_cdr(e));
			error_circuit(args);
			scm_stack_push(&args);

			if(scm_is_ffunc(func)) {
				Expr* toRet = scm_ffval(func)(args);
				scm_stack_pop(&args);
				scm_stack_pop(&func);

				scm_stack_pop(&e);
				return toRet;
			}
			
			if(!scm_is_closure(func)) {
				scm_stack_pop(&e);
				return scm_mk_error("can't evaluate (not a ffunc or closure)");
			}

			Expr* cenv = scm_closure_env(func);
			Expr* anames = scm_closure_args(func);
			Expr* body = scm_closure_body(func);

			//TODO check args are the same length as anames

			Expr* newEnv = scm_mk_env(cenv, anames, args);
			error_circuit(newEnv);
			CURRENT_ENV = newEnv;

			scm_stack_pop(&args);

			e = scm_mk_pair(BEGIN, body);

			scm_stack_pop(&func);

			if(!e) {
				scm_stack_pop(&e);
				return OOM;
			}
			goto begin;
		}

		scm_stack_pop(&e);
		return scm_mk_error("Can't evaluate pairs (yet)");
	} else if(scm_is_ffunc(e)) {
		scm_stack_pop(&e);
		return scm_mk_symbol("#(Foreign function)#");
	} else if(scm_is_symbol(e)) {
		scm_stack_pop(&e);
		return scm_env_lookup(CURRENT_ENV, e);
	} else if(scm_is_closure(e)) {
		scm_stack_pop(&e);
		return scm_mk_error("can't evaluate closure");
	} else {
		scm_stack_pop(&e);
		return e;
	}
}
