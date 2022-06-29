/*
 * Copyright ©️ 2022 Mario Forzanini <mf@marioforzanini.com>
 *
 * This file is part of derive.
 *
 * Derive is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Derive is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with derive. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "../dat.h"
#include "../fns.h"

START_TEST(test_l_init_non_exist)
{
	Lexer *l;

	l = l_init("non_existent.txt");
	ck_assert_ptr_null(l);
}
END_TEST

START_TEST(test_l_init_exist)
{
	Lexer *l;

	l = l_init("test_lex.txt");
	ck_assert_ptr_nonnull(l);
	ck_assert_str_eq("test_lex.txt", l->filename);
	ck_assert_uint_eq(l->line, 1);
	ck_assert_ptr_eq(l->pos, l->data);
	ck_assert_ptr_null(l->err);
	ck_assert_ptr_nonnull(l->pos);
	ck_assert_ptr_nonnull(l->data);

	l_cleanup(l);
}
END_TEST

START_TEST(test_lex_lparen)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("test_lex_lparen.txt");

	le = lex(l);
	ck_assert(le->type == LE_LPAREN);
	ck_assert_str_eq(le->lexeme, "(");

	free(le);
	free(le->lexeme);
	l_cleanup(l);
}
END_TEST

START_TEST(test_lex_number)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("test_lex_number.txt");

	le = lex(l);
	ck_assert(le->type == LE_NUMBER);
	ck_assert_str_eq(le->lexeme, "5.44");

	free(le);
	free(le->lexeme);
	l_cleanup(l);
}
END_TEST

START_TEST(test_lex_operator)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("test_lex_operator.txt");

	le = lex(l);
	ck_assert(le->type == LE_OPERATOR);
	ck_assert_str_eq(le->lexeme, "*");

	free(le);
	free(le->lexeme);
	l_cleanup(l);
}
END_TEST

START_TEST(test_lex_rparen)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("test_lex_rparen.txt");

	le = lex(l);
	ck_assert(le->type == LE_RPAREN);
	ck_assert_str_eq(le->lexeme, ")");

	free(le);
	free(le->lexeme);
	l_cleanup(l);
}
END_TEST

START_TEST(test_lex_symbol)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("test_lex_symbol.txt");

	le = lex(l);
	ck_assert(le->type == LE_SYMBOL);
	ck_assert_str_eq(le->lexeme, "sin");

	free(le);
	free(le->lexeme);
	l_cleanup(l);
}
END_TEST

START_TEST(test_lex_full)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("test_lex.txt");

	le = lex(l);
	ck_assert(le->type == LE_SYMBOL);
	ck_assert_str_eq(le->lexeme, "sin");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_LPAREN);
	ck_assert_str_eq(le->lexeme, "(");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_SYMBOL);
	ck_assert_str_eq(le->lexeme, "x");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_OPERATOR);
	ck_assert_str_eq(le->lexeme, "+");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_NUMBER);
	ck_assert_str_eq(le->lexeme, "2.5");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_RPAREN);
	ck_assert_str_eq(le->lexeme, ")");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_OPERATOR);
	ck_assert_str_eq(le->lexeme, "-");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_SYMBOL);
	ck_assert_str_eq(le->lexeme, "y");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_OPERATOR);
	ck_assert_str_eq(le->lexeme, "*");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_NUMBER);
	ck_assert_str_eq(le->lexeme, "5");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_OPERATOR);
	ck_assert_str_eq(le->lexeme, "/");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_NUMBER);
	ck_assert_str_eq(le->lexeme, "12");
	free(le);
	free(le->lexeme);

	l_cleanup(l);
}
END_TEST


Suite*
parse_suite(void)
{
	Suite *s;
	TCase *tc_lex;

	s = suite_create("parse");

	tc_lex = tcase_create("lex");

	tcase_add_test(tc_lex, test_l_init_non_exist);
	tcase_add_test(tc_lex, test_l_init_exist);
	tcase_add_test(tc_lex, test_lex_full);
	tcase_add_test(tc_lex, test_lex_lparen);
	tcase_add_test(tc_lex, test_lex_number);
	tcase_add_test(tc_lex, test_lex_operator);
	tcase_add_test(tc_lex, test_lex_rparen);
	tcase_add_test(tc_lex, test_lex_symbol);

	suite_add_tcase(s, tc_lex);

	return s;
}

int
main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = parse_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
