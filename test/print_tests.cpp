#include "Scheme.h"
#include "SchemeSecret.h"

#include <gtest/gtest.h>

TEST(Printing, Atoms) {
	scm_init();
	char* s;
	
	s = scm_print(scm_read("42"));
	EXPECT_STREQ("42", s);
	free(s);

	s = scm_print(scm_read("42.0"));
	EXPECT_STREQ("42.000000", s);
	free(s);

	s = scm_print(scm_read("#\\c"));
	EXPECT_STREQ("#\\c", s);
	free(s);

	s = scm_print(scm_read("#\\newline"));
	EXPECT_STREQ("#\\newline", s);
	free(s);
	
	s = scm_print(scm_read("#\\space"));
	EXPECT_STREQ("#\\space", s);
	free(s);
	
	s = scm_print(scm_read("#t"));
	EXPECT_STREQ("#t", s);
	free(s);
	
	s = scm_print(scm_read("#f"));
	EXPECT_STREQ("#f", s);
	free(s);

	s = scm_print(scm_read("\"string\""));
	EXPECT_STREQ("\"string\"", s);
	free(s);

	s = scm_print(scm_read("symbol"));
	EXPECT_STREQ("symbol", s);
	free(s);

	s = scm_print(scm_read("()"));
	EXPECT_STREQ("()", s);
	free(s);

	scm_reset();
}

TEST(Printing, Pairs) {
	scm_init();
	char* s;

	s = scm_print(scm_read("(() . ())"));
	EXPECT_STREQ("(())", s);
	free(s);

	s = scm_print(scm_read("(() . 42.0)"));
	EXPECT_STREQ("(() . 42.000000)", s);
	free(s);

	s = scm_print(scm_read("(42.0 . ())"));
	EXPECT_STREQ("(42.000000)", s);
	free(s);

	s = scm_print(scm_read("(a . b)"));
	EXPECT_STREQ("(a . b)", s);
	free(s);
	
	s = scm_print(scm_read("(a . (b . c))"));
	EXPECT_STREQ("(a b . c)", s);
	free(s);

	scm_reset();
}

TEST(Printing, Lists) {
	scm_init();
	char* s;
	
	s = scm_print(scm_read("(a 1 1.0 #t #\\c)"));
	EXPECT_STREQ("(a 1 1.000000 #t #\\c)", s);
	free(s);

	s = scm_print(scm_read("((a b) (c d))"));
	EXPECT_STREQ("((a b) (c d))", s);
	free(s);

	s = scm_print(scm_read("(())"));
	EXPECT_STREQ("(())", s);
	free(s);
	
	scm_reset();
}

TEST(Printing, Quotes) {
	scm_init();
	char *s;
	
	s = scm_print(scm_read("'22"));
	EXPECT_STREQ("(quote 22)", s);
	free(s);
	
	s = scm_print(scm_read("(quote '(quote a))"));
	EXPECT_STREQ("(quote (quote (quote a)))", s);
	free(s);

	scm_reset();
}
