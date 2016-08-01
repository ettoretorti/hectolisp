#include "Scheme.h"
#include "SchemeSecret.h"

#include <gtest/gtest.h>

TEST(Memory, CheckAllocation) {
	scm_init();
	
	Expr* i = scm_mk_int(42);
	EXPECT_TRUE(scm_is_atom(i));
	EXPECT_TRUE(scm_is_int(i));
	EXPECT_EQ(scm_ival(i), 42);

	Expr* r = scm_mk_real(42.0);
	EXPECT_TRUE(scm_is_atom(i));
	EXPECT_TRUE(scm_is_real(r));
	EXPECT_EQ(scm_rval(r), 42.0);

	Expr* c = scm_mk_char('E');
	EXPECT_TRUE(scm_is_atom(i));
	EXPECT_TRUE(scm_is_char(c));
	EXPECT_EQ(scm_cval(c), 'E');

	Expr* sym = scm_mk_symbol("symbol");
	EXPECT_TRUE(scm_is_atom(i));
	EXPECT_TRUE(scm_is_symbol(sym));
	EXPECT_STREQ(scm_sval(sym), "symbol");

	Expr* str = scm_mk_string("string");
	EXPECT_TRUE(scm_is_atom(i));
	EXPECT_TRUE(scm_is_string(str));
	EXPECT_STREQ(scm_sval(str), "string");

	Expr* p = scm_mk_pair(i, r);
	EXPECT_TRUE(scm_is_pair(p));
	EXPECT_EQ(scm_car(p), i);
	EXPECT_EQ(scm_cdr(p), r);

	scm_reset();
}

TEST(Memory, SymbolUniqueness) {
	scm_init();

	Expr* fst = scm_mk_symbol("symbol");
	Expr* snd = scm_mk_symbol("symbol");
	EXPECT_EQ(fst, snd);
	
	Expr* ascii[128];
	char buf[2] = { 0 };

	for(int i = 0; i < 128; i++) {
		buf[0] = (char) i;
		ascii[i] = scm_mk_symbol(buf);
	}

	for(int i = 0; i < 128; i++) {
		buf[0] = (char) i;
		EXPECT_EQ(scm_mk_symbol(buf), ascii[i]);
	}

	scm_reset();
}

TEST(Memory, CheckCorruption) {
	scm_init();
	
	Expr* es[500];
	for(int i=0; i<500; i++) {
		es[i] = scm_mk_int(i);
		EXPECT_TRUE(scm_is_int(es[i]));
		EXPECT_EQ(i, scm_ival(es[i]));
	}
	
	//check for corruption of older allocations by more recent ones
	for(int i=0; i<500; i++) {
		EXPECT_TRUE(scm_is_int(es[i]));
		EXPECT_EQ(i, scm_ival(es[i]));
	}
	
	scm_reset();
}
