#include "Scheme.h"
#include "SchemeSecret.h"

#include <gtest/gtest.h>

TEST(Eval, SelfEvaluating) {
	scm_init();
	char* s;

	s = scm_print(scm_eval(scm_read("42")));
	EXPECT_STREQ("42", s);
	free(s);

	s = scm_print(scm_eval(scm_read("42.0")));
	EXPECT_STREQ("42.000000", s);
	free(s);

	s = scm_print(scm_eval(scm_read("#\\c")));
	EXPECT_STREQ("#\\c", s);
	free(s);

	s = scm_print(scm_eval(scm_read("#\\newline")));
	EXPECT_STREQ("#\\newline", s);
	free(s);

	s = scm_print(scm_eval(scm_read("#\\space")));
	EXPECT_STREQ("#\\space", s);
	free(s);

	s = scm_print(scm_eval(scm_read("#t")));
	EXPECT_STREQ("#t", s);
	free(s);

	s = scm_print(scm_eval(scm_read("#f")));
	EXPECT_STREQ("#f", s);
	free(s);

	s = scm_print(scm_eval(scm_read("\"string\"")));
	EXPECT_STREQ("\"string\"", s);
	free(s);

	s = scm_print(scm_read("()"));
	EXPECT_STREQ("()", s);
	free(s);

	scm_reset();
}

TEST(Eval, Quotes) {
	scm_init();
	char* s;

	s = scm_print(scm_eval(scm_read("(quote abc)")));
	EXPECT_STREQ("abc", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(quote (quote 32))")));
	EXPECT_STREQ("(quote 32)", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(quote (wow (what (nesting (9000)))))")));
	EXPECT_STREQ("(wow (what (nesting (9000))))", s);
	free(s);

	scm_reset();
}

TEST(Eval, And) {
	scm_init();
	char* s;

	s = scm_print(scm_eval(scm_read("(and)")));
	EXPECT_STREQ("#t", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(and #f)")));
	EXPECT_STREQ("#f", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(and 1 #f)")));
	EXPECT_STREQ("#f", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(and (and) 3)")));
	EXPECT_STREQ("3", s);
	free(s);

	scm_reset();
}

TEST(Eval, Or) {
	scm_init();
	char* s;

	s = scm_print(scm_eval(scm_read("(or)")));
	EXPECT_STREQ("#f", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(or #f)")));
	EXPECT_STREQ("#f", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(or 1 #f)")));
	EXPECT_STREQ("1", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(or (or) 3)")));
	EXPECT_STREQ("3", s);
	free(s);

	scm_reset();
}

TEST(Eval, SimpleLambda) {
	scm_init();
	char* s;

	s = scm_print(scm_eval(scm_read("((lambda (x) x) 3)")));
	EXPECT_STREQ("3", s);
	free(s);

	s = scm_print(scm_eval(scm_read("((lambda (x y) y) 3 15)")));
	EXPECT_STREQ("15", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(((lambda (x) (lambda (y) y)) 3) 15)")));
	EXPECT_STREQ("15", s);
	free(s);

	s = scm_print(scm_eval(scm_read("((lambda (x) (+ x 3)) 9)")));
	EXPECT_STREQ("12", s);
	free(s);

	scm_reset();
}

TEST(Eval, If) {
	scm_init();
	char* s;

	s = scm_print(scm_eval(scm_read("(if #f 1 0)")));
	EXPECT_STREQ("0", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(if #t 1 0)")));
	EXPECT_STREQ("1", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(if 1 1 0)")));
	EXPECT_STREQ("1", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(if 1.0 1 0)")));
	EXPECT_STREQ("1", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(if () 1 0)")));
	EXPECT_STREQ("1", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(if \"test\" 1 0)")));
	EXPECT_STREQ("1", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(if 'wow 1 0)")));
	EXPECT_STREQ("1", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(if '(test) 1 0)")));
	EXPECT_STREQ("1", s);
	free(s);

	s = scm_print(scm_eval(scm_read("(if (and (+ 2 3) #f) (+ 1 4) (+ 2 2))")));
	EXPECT_STREQ("4", s);
	free(s);

	scm_reset();
}

TEST(Eval, TailCallOptimization) {
	scm_init();
	char* s;

	s = scm_print(scm_eval(scm_read("(begin (define tst (lambda (x) (if (= x 0) 0 (tst (+ x -1))))) (tst 2048))")));
	EXPECT_STREQ("0", s);
	free(s);

	scm_reset();
}
