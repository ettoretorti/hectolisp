#include "Scheme.h"
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

static Expr EOI_STRING = { .tag = ATOM, .atom = { .type = ERROR, .sval = "End of input before closing \" in string" }, .mark = true, .protect = true };
static Expr NON_DIGIT = { .tag = ATOM, .atom = { .type = ERROR, .sval = "Found non digit character in number" }, .mark = true, .protect = true };
static Expr EXPECTED_RPAREN = { .tag = ATOM, .atom = { .type = ERROR, .sval = "Couldn't find expected closing ) for list" }, .mark = true, .protect = true };
static Expr AFTER_HASH = { .tag = ATOM, .atom = { .type = ERROR, .sval = "Didn't find expected boolean or character after #" }, .mark = true, .protect = true };
static Expr UNEXPECTED_RPAREN = { .tag = ATOM, .atom = { .type = ERROR, .sval = "Unexpected ')'" }, .mark = true, .protect = true };
static Expr EOI = { .tag = ATOM, .atom = { .type = ERROR, .sval = "End of input" }, .mark = true, .protect = true };
static Expr UNKNOWN = { .tag = ATOM, .atom = { .type = ERROR, .sval = "Unknown error" }, .mark = true, .protect = true };

//for reading strings and symbols, max size is 10kb
static char buf[10240];

struct Buffer {
	const char* const s;
	size_t i;
	const size_t n;
};
typedef struct Buffer Buffer;

static void b_eat_white(Buffer* b) {
	size_t i = b->i;

	while(i < b->n && isspace(b->s[i])) {
		i++;
	}
	b->i = i;
}

static inline bool is_bound(char c) {
	return c == '\0'
	    || c == '('
	    || c == ')'
	    || c == ';'
	    || c == '\''
	    || c == '"'
	    || c == '`'
	    || c == '|'
	    || c == ' '
	    || c == '\t'
	    || c == '\n';
}

static void b_eat_til_bound(Buffer* b) {
	size_t i = b->i;

	while(i <= b->n && !is_bound(b->s[i])) {
		i++;
	}
	b->i = i;
}

static inline char b_get(Buffer* b) {
	if(b->i <= b->n) return b->s[b->i++];
	else            return '\0';
}

static inline char b_peek(const Buffer* b) {
	if(b->i <= b->n) return b->s[b->i];
	else            return '\0';
}

static inline void b_unget(Buffer* b) {
	assert(b->i > 0);
	b->i--;
}

static Expr* read_char(Buffer* b) {
	char c = b_get(b);
	char p = b_peek(b);
	
	if(c == 'n' && p == 'e') {
		b_eat_til_bound(b);
		c = '\n';
	} else if(c == 's' && p == 'p') {
		b_eat_til_bound(b);
		c = ' ';
	}

	return scm_mk_char(c);
}

static Expr* read_string(Buffer* b) {
	char c = b_get(b);
	assert(c == '"');

	size_t idx = 0;

	while(true) {
		assert(idx < 10240);

		c = b_get(b);
		if(c == '\0') return &EOI_STRING;
		if(c == '"') {
			buf[idx] = '\0';
			break;
		}

		if(c == '\\') {
			c = b_get(b);
			if(c == '\0') return &EOI_STRING;

			c = c == 'n' ? '\n' :
			    c == 't' ? '\t' :
			                c;
		}

		buf[idx++] = c;
	}

	return scm_mk_string(buf);
}

static Expr* read_symbol(Buffer* b) {
	size_t idx = 0;

	while(true) {
		assert(idx < 10240);
		
		char c = b_get(b);

		if(is_bound(c)) {
			b_unget(b);
			buf[idx] = '\0';
			return scm_mk_symbol(buf);
		}

		buf[idx++] = c;
	}
}

static Expr* read_num(Buffer* b, int sign) {
	double buf = 0.0;
	long long lbuf = 0;
	double nDecimals = 1.0;
	bool postDec = false;

	while(true) {
		char c = b_get(b);

		if(is_bound(c)) {
			b_unget(b);
			break;
		}
		
		if(c == '.') {
			postDec = true;
			continue;
		}

		if(!isdigit(c)) {
			return &NON_DIGIT;
		}

		if(!postDec) {
			buf *= 10;
			buf += c - '0';

			lbuf *= 10;
			lbuf += c - '0';
		} else {
			nDecimals /= 10;
			buf += ((double) c - '0') * nDecimals;
		}
	}
	
	return postDec ? scm_mk_real(buf * sign) : scm_mk_int(lbuf * sign);
}

static Expr* reade(Buffer* b);

Expr* reade_list(Buffer* b) {
	char c = b_get(b);
	assert(c == '(');

	b_eat_white(b);
	c = b_peek(b);

	if(c == ')') {
		b_get(b);
		return EMPTY_LIST;
	}

	Expr* read = reade(b);
	if(scm_is_error(read)) return read;
	Expr* car = scm_mk_pair(read, EMPTY_LIST);
	Expr* toRet = car;

	while(true) {
		b_eat_white(b);
		if(b_peek(b) == '.') {
			b_get(b);
			car->pair.cdr = reade(b);
			b_eat_white(b);
			if(b_get(b) != ')') {
				return &EXPECTED_RPAREN;
			}
			break;
		} else if(b_peek(b) == ')') {
			b_get(b);
			return toRet;
		} else {
			read = reade(b);
			if(scm_is_error(read)) return read;
			Expr* ncdr = scm_mk_pair(read, EMPTY_LIST);
			car->pair.cdr = ncdr;
			car = ncdr;
		}
	}

	return toRet;
}

static Expr* reade(Buffer* b) {
	b_eat_white(b);

	char cur = b_peek(b);
	Expr* final = EMPTY_LIST;

	if(cur == '(') {
		final = reade_list(b);
	} else if(isdigit(cur)) {
		final = read_num(b, 1);
	} else if(cur == '-') {
		b_get(b);
		char c = b_peek(b);

		if(isdigit(c)) {
			final = read_num(b, -1);
		} else {
			b_unget(b);
			final = read_symbol(b);
		}
	} else if(cur == '"') {
		final = read_string(b);
	} else if(cur == '#') {
		b_get(b);
		char c = b_get(b);
		
		final = c == 't'  ? TRUE :
			c == 'f'  ? FALSE :
			c == '\\' ? read_char(b) :
			            &AFTER_HASH;
	} else if(!is_bound(cur) && !isdigit(cur)) {
		final = read_symbol(b);
	} else if(cur == ')') {
		final = &UNEXPECTED_RPAREN;
	} else if(cur == '\0') {
		final = &EOI;
	} else {
		final = &UNKNOWN;
	}

	return final;
}

Expr* scm_read(const char* in) {
	Buffer b = { .s = in, .i = 0, .n = strlen(in) };

	return reade(&b);
}
