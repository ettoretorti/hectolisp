#include "Scheme.h"
#include "SchemeSecret.h"

#include <gtest/gtest.h>

TEST(Parsing, Atoms) {
	scm_init();

	Expr* i = scm_read("42");
	EXPECT_TRUE(scm_is_int(i));
	EXPECT_EQ(42, scm_ival(i));

	Expr* r = scm_read("42.0");
	EXPECT_TRUE(scm_is_real(r));
	EXPECT_EQ(42.0, scm_rval(r));

	Expr* c = scm_read("#\\c");
	EXPECT_TRUE(scm_is_char(c));
	EXPECT_EQ('c', scm_cval(c));

	Expr* n = scm_read("#\\newline");
	EXPECT_TRUE(scm_is_char(n));
	EXPECT_EQ('\n', scm_cval(n));

	Expr* s = scm_read("#\\space");
	EXPECT_TRUE(scm_is_char(s));
	EXPECT_EQ(' ', scm_cval(s));

	Expr* t = scm_read("#t");
	EXPECT_TRUE(scm_is_bool(t));
	EXPECT_TRUE(scm_bval(t));
	EXPECT_EQ(TRUE, t);

	Expr* f = scm_read("#f");
	EXPECT_TRUE(scm_is_bool(f));
	EXPECT_FALSE(scm_bval(f));
	EXPECT_EQ(FALSE, f);

	Expr* str = scm_read("\"string\"");
	EXPECT_TRUE(scm_is_string(str));
	EXPECT_STREQ("string", scm_sval(str));

	Expr* sym = scm_read("symbol");
	EXPECT_TRUE(scm_is_symbol(sym));
	EXPECT_STREQ("symbol", scm_sval(sym));

	Expr* e = scm_read("()");
	EXPECT_EQ(EMPTY_LIST, e);

	scm_reset();
}

TEST(Parsing, Pairs) {
	scm_init();

	Expr* a = scm_read("(() . ())");
	EXPECT_TRUE(scm_is_pair(a));
	EXPECT_EQ(EMPTY_LIST, scm_car(a));
	EXPECT_EQ(EMPTY_LIST, scm_cdr(a));

	Expr* b = scm_read("(() . 42.0)");
	EXPECT_TRUE(scm_is_pair(b));
	EXPECT_EQ(EMPTY_LIST, scm_car(b));
	EXPECT_TRUE(scm_is_real(scm_cdr(b)));
	EXPECT_EQ(42.0, scm_rval(scm_cdr(b)));

	Expr* c = scm_read("(42.0 . ())");
	EXPECT_TRUE(scm_is_pair(c));
	EXPECT_TRUE(scm_is_real(scm_car(c)));
	EXPECT_EQ(42.0, scm_rval(scm_car(c)));
	EXPECT_EQ(EMPTY_LIST, scm_cdr(c));

	Expr* d = scm_read("(a . b)");
	EXPECT_TRUE(scm_is_pair(d));
	EXPECT_TRUE(scm_is_symbol(scm_car(d)));
	EXPECT_STREQ("a", scm_sval(scm_car(d)));
	EXPECT_TRUE(scm_is_symbol(scm_cdr(d)));
	EXPECT_STREQ("b", scm_sval(scm_cdr(d)));

	Expr* e = scm_read("(a . (b . c))");
	EXPECT_TRUE(scm_is_pair(e));
	EXPECT_TRUE(scm_is_symbol(scm_car(e)));
	EXPECT_STREQ("a", scm_sval(scm_car(e)));
	Expr* f = scm_cdr(e);
	EXPECT_TRUE(scm_is_pair(f));
	EXPECT_TRUE(scm_is_symbol(scm_car(f)));
	EXPECT_STREQ("b", scm_sval(scm_car(f)));
	EXPECT_TRUE(scm_is_symbol(scm_cdr(f)));
	EXPECT_STREQ("c", scm_sval(scm_cdr(f)));

	scm_reset();
}

TEST(Parsing, Lists) {
	scm_init();
	
	Expr* a = scm_read("(a 1 #t)");
	EXPECT_TRUE(scm_is_pair(a));
	EXPECT_EQ(scm_mk_symbol("a"), scm_car(a));
	
	EXPECT_TRUE(scm_is_int(scm_cadr(a)));
	EXPECT_EQ(1, scm_ival(scm_cadr(a)));
	
	EXPECT_TRUE(scm_is_bool(scm_caddr(a)));
	EXPECT_EQ(TRUE, scm_caddr(a));

	EXPECT_EQ(EMPTY_LIST, scm_cdddr(a));


	Expr* b = scm_read("((a b) (c d))");
	EXPECT_TRUE(scm_is_pair(b));

	Expr* c = scm_car(b);
	EXPECT_TRUE(scm_is_pair(c));
	EXPECT_EQ(scm_mk_symbol("a"), scm_car(c));

	EXPECT_TRUE(scm_is_pair(scm_cdr(c)));
	EXPECT_EQ(scm_mk_symbol("b"), scm_cadr(c));

	EXPECT_EQ(EMPTY_LIST, scm_cddr(c));

	Expr* d = scm_cadr(b);
	EXPECT_TRUE(scm_is_pair(d));
	EXPECT_EQ(scm_mk_symbol("c"), scm_car(d));
	
	EXPECT_TRUE(scm_is_pair(scm_cdr(d)));
	EXPECT_EQ(scm_mk_symbol("d"), scm_cadr(d));

	EXPECT_EQ(EMPTY_LIST, scm_cddr(d));

	EXPECT_EQ(EMPTY_LIST, scm_cddr(b));


	Expr* e = scm_read("(())");
	EXPECT_TRUE(scm_is_pair(e));
	EXPECT_EQ(EMPTY_LIST, scm_car(e));
	EXPECT_EQ(EMPTY_LIST, scm_cdr(e));

	scm_reset();
}

TEST(Parsing, Quotes) {
	scm_init();

	Expr* a = scm_read("'22");
	EXPECT_TRUE(scm_is_pair(a));
	EXPECT_EQ(scm_car(a), scm_mk_symbol("quote"));
	EXPECT_TRUE(scm_is_pair(scm_cdr(a)));
	EXPECT_TRUE(scm_is_int(scm_cadr(a)));
	EXPECT_EQ(scm_ival(scm_cadr(a)), 22);

	EXPECT_EQ(scm_cddr(a), EMPTY_LIST);

	Expr* b = scm_read("(quote '(quote a))");

	for(int i = 0; i < 3; i++) {
		EXPECT_TRUE(scm_is_pair(b));
		EXPECT_EQ(scm_mk_symbol("quote"), scm_car(b));
		EXPECT_TRUE(scm_is_pair(scm_cdr(b)));
		EXPECT_EQ(EMPTY_LIST, scm_cddr(b));
		b = scm_cadr(b);
	}

	EXPECT_EQ(scm_mk_symbol("a"), b);

	scm_reset();
}

TEST(Parsing, Incremental) {
	scm_init();

	char toRead[] = "a b c";
	char* ptr = toRead;
	
	Expr* e = scm_read_inc(ptr, &ptr);
	EXPECT_EQ(scm_mk_symbol("a"), e);
	e = scm_read_inc(ptr, &ptr);
	EXPECT_EQ(scm_mk_symbol("b"), e);
	e = scm_read_inc(ptr, &ptr);
	EXPECT_EQ(scm_mk_symbol("c"), e);
	e = scm_read_inc(ptr, &ptr);
	EXPECT_EQ('\0', *ptr);

	scm_reset();
}
