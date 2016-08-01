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

	Expr* g = scm_read("((a . b) . c)");

	scm_reset();
}

TEST(Parsing, Lists) {
	scm_init();

	scm_reset();
}
