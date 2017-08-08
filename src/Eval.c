#include "Scheme.h"
#include "SchemeSecret.h"

#include <assert.h>

static Expr* stc_eval(Expr* e);

static bool is_tpair(Expr* e, Expr* tag) {
	assert(e); assert(tag); assert(scm_is_symbol(tag));

	return scm_is_pair(e) && scm_is_symbol(scm_car(e)) && scm_car(e) == tag;
}

static inline bool is_last(Expr* l) {
	assert(scm_is_pair(l));

	return !scm_is_pair(scm_cdr(l));
}

static Expr* quote_all_inplace(Expr* l) {
	scm_stack_push(&l);
	Expr* cur = l;
	while(scm_is_pair(cur)) {
		Expr* ll[2] = { QUOTE, scm_car(cur) };
		cur->pair.car = scm_mk_list(ll, 2);
		if(scm_is_error(scm_car(cur))) {
			scm_stack_pop(&l);
			return scm_car(cur);
		}
		cur = scm_cdr(cur);
	}

	scm_stack_pop(&l);
	return l;
}

static Expr* save_eval(Expr* es) {
	Expr* curEnv = CURRENT_ENV;
	scm_stack_push(&curEnv);

	Expr* res = stc_eval(es);

	CURRENT_ENV = curEnv;
	scm_stack_pop(&curEnv);

	return res;
}

static Expr* def2lambda(Expr* defBody) {
	assert(defBody);
	assert(scm_is_pair(defBody));

	Expr* nameArgs = scm_car(defBody);
	Expr* body = scm_cdr(defBody);

	assert(scm_is_pair(nameArgs));

	Expr* args = scm_cdr(nameArgs);

	Expr* tmp = scm_mk_pair(args, body);
	if(!tmp) return OOM;

	scm_stack_push(&tmp);

	tmp = scm_mk_pair(LAMBDA, tmp);
	if(!tmp) tmp = OOM;

	scm_stack_pop(&tmp);

	return tmp;
}

// Requires that the two elements in dest are protected
static bool decomposeBindings(Expr* bindings, Expr* dest[2]) {
	assert(bindings); assert(dest);
	if(bindings == EMPTY_LIST) {
		dest[0] = dest[1] = EMPTY_LIST;
		return true;
	} else if(scm_is_pair(bindings)) {
		if(!decomposeBindings(scm_cdr(bindings), dest)) return false;

		Expr* curBinding = scm_car(bindings);
		if(!scm_is_pair(curBinding)) return false;

		Expr* name = scm_car(curBinding);
		if(!scm_is_symbol(name)) return false;

		Expr* val = scm_cdr(curBinding);
		if(!scm_is_pair(val)) return false;
		if(scm_cdr(val) != EMPTY_LIST) return false;

		val = scm_car(val);

		dest[0] = scm_mk_pair(name, dest[0]);
		if(!dest[0]) {
			dest[0] = EMPTY_LIST;
			return false;
		}

		dest[1] = scm_mk_pair(val, dest[1]);
		if(!dest[1]) {
			dest[1] = EMPTY_LIST;
			return false;
		}

		return true;
	} else {
		return false;
	}
}

static Expr* namedlet2lambda(Expr* name, Expr* rest) {
	if(!scm_is_pair(rest)) {
		return scm_mk_error("malformed named let");
	}
	Expr* bindings = scm_car(rest);
	rest = scm_cdr(rest);
	if(!scm_is_pair(rest)) {
		return scm_mk_error("malformed named let body");
	}

	Expr* decomp[2] = { EMPTY_LIST, EMPTY_LIST };
	scm_stack_push(&decomp[0]);
	scm_stack_push(&decomp[1]);

	Expr* toRet = EMPTY_LIST;
	scm_stack_push(&toRet);
	if(!decomposeBindings(bindings, decomp)) {
		toRet = scm_mk_error("malformed named let bindings");
	} else {
		Expr* ll[4];

		Expr* call = EMPTY_LIST;
		scm_stack_push(&call);
		ll[0] = name;
		ll[1] = decomp[1];
		call = scm_concat(ll, 2);

		Expr* inlambda = EMPTY_LIST;
		scm_stack_push(&inlambda);
		ll[0] = LAMBDA;
		ll[1] = decomp[0];
		ll[2] = rest;
		inlambda = scm_concat(ll, 3);

		Expr* define = EMPTY_LIST;
		scm_stack_push(&define);
		ll[0] = DEFINE;
		ll[1] = name;
		ll[2] = inlambda;
		define = scm_mk_list(ll, 3);

		ll[0] = LAMBDA;
		ll[1] = EMPTY_LIST;
		ll[2] = define;
		ll[3] = call;
		toRet = scm_mk_list(ll, 4);
		toRet = scm_mk_list(&toRet, 1);

		scm_stack_pop(&define);
		scm_stack_pop(&inlambda);
		scm_stack_pop(&call);
	}

	scm_stack_pop(&toRet);
	scm_stack_pop(&decomp[1]);
	scm_stack_pop(&decomp[0]);

	return toRet;
}

static Expr* let2lambda(Expr* let) {
	assert(let);
	assert(scm_is_pair(let));
	assert(scm_car(let) == LET);

	Expr* next = scm_cdr(let);
	if(!scm_is_pair(next)) {
		return scm_mk_error("malformed let");
	}
	Expr* bindings = scm_car(next);
	next = scm_cdr(next);

	if(scm_is_symbol(bindings)) {
		return namedlet2lambda(bindings, next);
	}

	if(!scm_is_pair(next)) {
		return scm_mk_error("malformed let");
	}

	if(bindings == EMPTY_LIST) {
		Expr* toRet = next;
		scm_stack_push(&toRet);

		Expr* ll[3] = { LAMBDA, EMPTY_LIST, next };
		toRet = scm_concat(ll, 3);
		toRet = scm_mk_list(&toRet, 1);

		scm_stack_pop(&toRet);
		return toRet;
	}

	if(!scm_is_pair(bindings)) {
		return scm_mk_error("malformed let bindings");
	}

	Expr* decomp[2] = { EMPTY_LIST, EMPTY_LIST };
	scm_stack_push(&decomp[0]);
	scm_stack_push(&decomp[1]);

	Expr* toRet = EMPTY_LIST;
	scm_stack_push(&toRet);
	if(!decomposeBindings(bindings, decomp)) {
		toRet = scm_mk_error("malformed let bindings");
	} else {
		Expr* ll[3] = { LAMBDA, decomp[0], next };
		toRet = scm_concat(ll, 3);
		toRet = scm_mk_pair(toRet, decomp[1]);
		toRet = toRet ? toRet : OOM;
	}

	scm_stack_pop(&toRet);
	scm_stack_pop(&decomp[1]);
	scm_stack_pop(&decomp[0]);

	return toRet;
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
		cur->pair.car = stc_eval(scm_car(es));
		if(scm_is_error(scm_car(cur))) {
			scm_stack_pop(&head); scm_stack_pop(&curEnv); CURRENT_ENV = curEnv;
			return scm_car(cur);
		}

		if(scm_is_pair(scm_cdr(es))) {
			cur->pair.cdr = scm_mk_pair(EMPTY_LIST, EMPTY_LIST);
			if(!cur->pair.cdr) {
				scm_stack_pop(&head); scm_stack_pop(&curEnv); CURRENT_ENV = curEnv;
				return OOM;
			}
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

static Expr* quasi_eval(Expr* e, unsigned level) {
	if(is_tpair(e, QUASIQUOTE)) {
		Expr* rest = scm_cdr(e);
		if(!scm_is_pair(rest)) {
			return scm_mk_error("Lonely nested quasiquote");
		}

		Expr* res =  quasi_eval(scm_car(rest), level + 1);
		if(scm_is_error(res)) return res;

		scm_stack_push(&res);
		Expr* ll[2] = { QUASIQUOTE, res };
		res = scm_mk_list(ll, 2);
		scm_stack_pop(&res);

		return res;
	} else if(is_tpair(e, UNQUOTE) || is_tpair(e, UNQUOTE_SPLICING)) {
		Expr* rest = scm_cdr(e);
		if(!scm_is_pair(rest)) {
			return scm_mk_error("Lonely unquote");
		}

		if(level == 0) {
			return save_eval(scm_car(rest));
		} else {
			Expr* res = quasi_eval(scm_car(rest), level - 1);
			if(scm_is_error(res)) return res;

			scm_stack_push(&res);
			Expr* ll[2] = { UNQUOTE, res };
			res = scm_mk_list(ll, 2);
			scm_stack_pop(&res);

			return res;
		}
	} else if(scm_is_pair(e)) {
		Expr* car = scm_car(e);
		Expr* cdr = scm_cdr(e);

		scm_stack_push(&car);
		scm_stack_push(&cdr);

		bool splice = is_tpair(car, UNQUOTE_SPLICING);
		car = quasi_eval(car, level);
		if(scm_is_error(car)) {
			scm_stack_pop(&cdr);
			scm_stack_pop(&car);
			return car;
		}
		cdr = quasi_eval(cdr, level);
		if(scm_is_error(cdr)) {
			scm_stack_pop(&cdr);
			scm_stack_pop(&car);
			return cdr;
		}

		Expr* toRet = splice ? scm_append(car, cdr) : scm_mk_pair(car, cdr);
		toRet = toRet ? toRet : OOM;

		scm_stack_pop(&cdr);
		scm_stack_pop(&car);

		return toRet;
	} else {
		return e;
	}
}

#undef error_circuit

// needs modification to deal with the tailcall cleanup
// TODO STILL BROKEN IN CASES OTHER THINGS ARE STACKED FIRST
#define error_circuit(expr) \
	{ Expr* TmP = expr; \
	  if(scm_is_error(TmP)) { scm_stack_pop(&e); return TmP; } }

static Expr* stc_eval(Expr* e) {
	scm_stack_push(&e);
//just a tail call
begin:
	assert(e);

	if(scm_is_pair(e)) {
		if(is_tpair(e, QUOTE)) {
			scm_stack_pop(&e);
			return scm_cadr(e);
		} else if(is_tpair(e, QUASIQUOTE)) {
			Expr* rest = scm_cdr(e);
			if(!scm_is_pair(rest)) {
				scm_stack_pop(&e);
				return scm_mk_error("Lonely quasiquote");
			}

			Expr* toRet = quasi_eval(scm_car(rest), 0);
			scm_stack_pop(&e);

			return toRet;
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
		} else if(is_tpair(e, LET)) {
			e = let2lambda(e);
			goto begin;
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

			bool isPair = scm_is_pair(e);

			if(isPair && scm_is_pair(scm_car(e))) {
				Expr* name = scm_caar(e);
				if(!scm_is_symbol(name)) {
					scm_stack_pop(&e);
					return scm_mk_error("name for defined function is not a symbol");
				}

				Expr* closure = stc_eval(def2lambda(e));
				error_circuit(closure);
				scm_stack_push(&closure);
				Expr* res = scm_env_define(CURRENT_ENV, name, closure);
				scm_stack_pop(&closure);

				scm_stack_pop(&e);
				return res;
			}

			if(!isPair || !scm_is_symbol(scm_car(e)) || !scm_is_pair(scm_cdr(e)) || scm_cddr(e) != EMPTY_LIST) {
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
			if(!scm_is_pair(args) && !scm_is_symbol(args) && args != EMPTY_LIST) {
				scm_stack_pop(&e);
				return scm_mk_error("lambda arguments aren't in a list and aren't a single symbol");
			}

			Expr* body = scm_cdr(e);
			if(!scm_is_pair(body)) {
				scm_stack_pop(&e);
				return scm_mk_error("lambda body is not a list");
			}

			scm_stack_pop(&e);
			return scm_mk_closure(CURRENT_ENV, args, body);
		} else  if(is_tpair(e, R_APPLY)) {
			e = scm_cdr(e);

			if(!scm_is_pair(e)) {
				scm_stack_pop(&e);
				return scm_mk_error("insufficient arguments to __apply");
			}
			Expr* func = scm_car(e);

			e = scm_cdr(e);
			if(!scm_is_pair(e)) {
				scm_stack_pop(&e);
				return scm_mk_error("insufficient arguments to __apply");
			}
			Expr* args = scm_car(e);

			scm_stack_push(&args);
			args = save_eval(args);
			if(scm_is_error(args)) {
				scm_stack_pop(&args); scm_stack_pop(&e);
				return args;
			}
			if(!scm_is_pair(args)) {
				scm_stack_pop(&args); scm_stack_pop(&e);
				return scm_mk_error("args to apply aren't a list");
			}
			args = quote_all_inplace(args);
			if(scm_is_error(args)) {
				scm_stack_pop(&args); scm_stack_pop(&e);
				return args;
			}

			// tail call
			e = scm_mk_pair(func, args);
			scm_stack_pop(&args);
			e = e ? e : OOM;
			goto begin;
		} else  if(is_tpair(e, R_EVAL)) {
			e = scm_cdr(e);
			if(!scm_is_pair(e)) {
				scm_stack_pop(&e);
				return scm_mk_error("insufficient arguments to __eval");
			}
			Expr* toEval = scm_car(e);

			toEval = save_eval(toEval);
			if(scm_is_error(toEval)) {
				scm_stack_pop(&e);
				return toEval;
			}

			e = scm_cdr(e);
			if(!scm_is_pair(e)) {
				scm_stack_pop(&e);
				return scm_mk_error("insufficient arguments to __eval");
			}
			Expr* env = scm_car(e);
			scm_stack_push(&toEval);
			env = save_eval(env);

			if(!scm_is_env(env) && env != FALSE) {
				scm_stack_pop(&toEval); scm_stack_pop(&e);
				if(scm_is_error(env)) return env;
				return scm_mk_error("invalid environment passed to __eval");
			}

			CURRENT_ENV = env;
			e = toEval;
			scm_stack_pop(&toEval);
			goto begin;
		} else {
			//TODO GC safety
			Expr* func = save_eval(scm_car(e));
			error_circuit(func);
			scm_stack_push(&func);

			Expr* args = save_eval_all(scm_cdr(e));
			if(scm_is_error(args)) {
				scm_stack_pop(&func); scm_stack_pop(&e);
				return args;
			}
			scm_stack_push(&args);

			if(scm_is_ffunc(func)) {
				Expr* toRet = scm_ffval(func)(args);
				scm_stack_pop(&args);
				scm_stack_pop(&func);

				scm_stack_pop(&e);
				return toRet;
			}

			if(!scm_is_closure(func)) {
				scm_stack_pop(&args); scm_stack_pop(&func); scm_stack_pop(&e);
				return scm_mk_error("can't evaluate (not a ffunc or closure)");
			}

			Expr* cenv = scm_closure_env(func);
			Expr* anames = scm_closure_args(func);
			Expr* body = scm_closure_body(func);

			Expr* newEnv = NULL;
			if(scm_is_pair(anames)) {
				int nlen = scm_list_len(anames);
				// args may be of the form (x y . zs)
				if(nlen == -1) {
					newEnv = scm_mk_env(cenv, EMPTY_LIST, EMPTY_LIST);
					Expr* rns;
					Expr* ras;
					for(rns = anames, ras = args; scm_is_pair(rns); rns = scm_cdr(rns), ras = scm_cdr(ras)) {
						if(ras == EMPTY_LIST) {
							newEnv = scm_mk_error("too few args to procedure");
							break;
						}
						scm_env_define_unsafe(newEnv, scm_car(rns), scm_car(ras));
					}

					if(scm_is_symbol(rns)) {
						scm_env_define_unsafe(newEnv, rns, ras);
					} else if(rns == EMPTY_LIST) {
						if(ras != EMPTY_LIST) {
							newEnv = scm_mk_error("too many args to procedure");
						}
					} else if(!scm_is_error(newEnv)) {
						newEnv = scm_mk_error("last entry in dotted tail args isn't a symbol");
					}
				} else {
					int alen = scm_list_len(args);
					newEnv = nlen == alen ? scm_mk_env(cenv, anames, args) : scm_mk_error("incorrect number of args to procedure");
				}
			} else {
				Expr* argname = EMPTY_LIST;
				Expr* flatargs = EMPTY_LIST;
				scm_stack_push(&argname);
				scm_stack_push(&flatargs);

				argname = scm_mk_pair(anames, EMPTY_LIST);
				flatargs = scm_mk_pair(args, EMPTY_LIST);

				newEnv = !(argname && flatargs) ? OOM : scm_mk_env(cenv, argname, flatargs);

				scm_stack_pop(&flatargs);
				scm_stack_pop(&argname);
			}

			if(scm_is_error(newEnv)) {
				scm_stack_pop(&args); scm_stack_pop(&func); scm_stack_pop(&e);
				return newEnv;
			}
			scm_stack_push(&newEnv);

			e = scm_mk_pair(BEGIN, body);

			scm_stack_pop(&newEnv);
			scm_stack_pop(&args);
			scm_stack_pop(&func);

			if(!e) {
				scm_stack_pop(&e);
				return OOM;
			}
			CURRENT_ENV = newEnv;
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

Expr* scm_eval(Expr* e) {
	assert(e);
	return save_eval(e);
}
