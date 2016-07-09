#include "Scheme.h"
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	char* buf;
	size_t size;
	size_t cap;
} buf;

static jmp_buf excpt;

static void reallocb(buf *b) {
	assert(b);
	size_t ncap = (size_t) (b->cap * 1.5);
	if(ncap < b->cap) {
		longjmp(excpt, 1);
	}

	char *nbuf = realloc(b->buf, ncap);
	if(!nbuf) {
		longjmp(excpt, 2);
	}

	b->buf = nbuf;
	b->cap = ncap;
}

static void append(buf* b, const char* s) {
	assert(b);
	assert(s);

	while(true) {
		while(*s && (b->size < b->cap)) {
			b->buf[b->size++] = *s++;
		}
		
		if(*s) reallocb(b);
		else  return;
	}
}

static void print_int(long long ival, buf* b) {
	assert(b);
	char lbuf[32];
	sprintf(lbuf,"%lld", ival);

	append(b, lbuf);
}

static void print_real(double rval, buf* b) {
	assert(b);
	char lbuf[128];
	sprintf(lbuf, "%f", rval);

	append(b, lbuf);
}

static void print_bool(bool bval, buf* b) {
	assert(b);

	append(b, bval ? "#t" : "#f");
}

static void print_char(char cval, buf* b) {
	assert(b);
	
	char cbuf[10] = "#\\newline";

	switch(cval) {
		case '\n':
			break;
		case ' ':
			strcpy(cbuf, "#\\space");
			break;
		default:
			cbuf[2] = cval;
			cbuf[3] = '\0';
			break;
	}

	append(b, cbuf);
}

static void print(Expr* e, buf* b);

static void print_pair(Expr* e, buf* b) {
	assert(b);
	assert(e);
	assert(scm_is_pair(e));

	if(e == EMPTY_LIST) {
		append(b, "()");
		return;
	}

	append(b, "(");

loop:   ;
	Expr* car = scm_car(e);
	Expr* cdr = scm_cdr(e);

	print(car, b);

	if(cdr != EMPTY_LIST) {
		if(scm_is_pair(cdr)) {
			append(b, " ");
			e = cdr;
			goto loop;
		} else {
			append(b, " . ");
			print(cdr, b);
		}
	}
	
	append(b, ")");
}

static void print(Expr* e, buf* b) {
	assert(e);
	assert(b);

	if(scm_is_pair(e)) {
		print_pair(e, b);
		return;
	}

	switch(e->atom.type) {
	case INT:
		print_int(scm_ival(e), b);
		break;
	case REAL:
		print_real(scm_rval(e), b);
		break;
	case BOOL:
		print_bool(scm_bval(e), b);
		break;
	case CHAR:
		print_char(scm_cval(e), b);
		break;
	case STRING:
		//TODO escaping
		append(b, "\"");
		append(b, scm_sval(e));
		append(b, "\"");
		break;
	case SYMBOL:
		append(b, scm_sval(e));
		break;
	default:
		append(b, "#UNKNOWN#");
		break;
	}
}

char* scm_print(Expr* e) {
	buf b = { .buf = malloc(128), .cap = 128, .size = 0 };
	
	int r;
	if(!(r = setjmp(excpt))) {
		print(e, &b);
	} else {
		free(b.buf);
		return NULL;
	}
	
	b.buf = realloc(b.buf, b.size+1);
	b.buf[b.size] = '\0';

	return b.buf;
}
