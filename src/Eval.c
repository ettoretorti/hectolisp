#include "Scheme.h"
#include "SchemeSecret.h"

#include <assert.h>

static bool is_tpair(Expr* e, Expr* tag) {
	assert(e); assert(tag); assert(scm_is_symbol(tag));

	return scm_is_pair(e) && scm_is_symbol(scm_car(e)) && scm_car(e) == tag;
}

Expr* scm_eval(Expr* e) {
//just a tail call
begin:
	assert(e);

	if(scm_is_pair(e)) {
		if(is_tpair(e, QUOTE)) {
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

			Expr* predEvaled = scm_eval(predicate);
			if(scm_is_error(predEvaled)) {
				return predEvaled;
			} else if(scm_is_true(predEvaled)) {
				e = consequent;
			} else {
				e = alternative;
			}
			goto begin;

			error:
				return scm_mk_error("Incorrect number of args to if (expected 3)");
		} else if(is_tpair(e, BEGIN)) {
			e = scm_cdr(e);
			
			Expr* last = EMPTY_LIST;
			while(scm_is_pair(e)) {
				last = scm_eval(scm_car(e));
				e = scm_cdr(e);
			}
			
			if(e != EMPTY_LIST) {
				return scm_mk_error("sequence of expressions to evaluate isn't a proper list");
			}

			return last;
		} else if(is_tpair(e, COND)) {
			e = scm_cdr(e);

			while(scm_is_pair(e)) {
				Expr* clause = scm_car(e);

				if(!scm_is_pair(clause) || !scm_is_pair(scm_cdr(clause))) {
					return scm_mk_error("Malformed cond clause");
				}

				Expr* predicate = scm_car(clause);

				bool go = predicate == ELSE;
				if(!go) {
					Expr* predEvaled = scm_eval(predicate);
					if(scm_is_error(predEvaled)) {
						return predEvaled;
					}

					go = scm_is_true(predEvaled);
				}

				if(go) {
					e = scm_mk_pair(BEGIN, scm_cdr(clause));
					goto begin;
				}

				e = scm_cdr(e);
			}

			if(e != EMPTY_LIST) {
				return scm_mk_error("sequence of clauses in cond isn't a proper list");
			}

			return EMPTY_LIST;
		} else if(is_tpair(e, DEFINE)) {
			e = scm_cdr(e);

			if(!scm_is_pair(e) || !scm_is_symbol(scm_car(e)) || !scm_is_pair(scm_cdr(e)) || scm_cddr(e) != EMPTY_LIST) {
				return scm_mk_error("Malformed define");
			}

			Expr* name = scm_car(e);
			Expr* val = scm_cadr(e);

			return scm_env_define(CURRENT_ENV, name, val);
		} else if(is_tpair(e, SET)) {
			e = scm_cdr(e);
			
			if(!scm_is_pair(e) || !scm_is_symbol(scm_car(e)) || !scm_is_pair(scm_cdr(e)) || scm_cddr(e) != EMPTY_LIST) {
				return scm_mk_error("Malformed set!");
			}

			Expr* name = scm_car(e);
			Expr* val = scm_cadr(e);

			return scm_env_set(CURRENT_ENV, name, val);
		}

		return scm_mk_error("Can't evaluate pairs (yet)");
	} else if(e == EMPTY_LIST) {
		return EMPTY_LIST;
	} else if(scm_is_ffunc(e)) {
		return scm_mk_symbol("#(Foreign function)#");
	} else if(scm_is_symbol(e)) {
		return scm_env_lookup(CURRENT_ENV, e);
	} else {
		return e;
	}
}
