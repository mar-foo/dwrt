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

	l = l_init("files/non_existent.txt");
	ck_assert_ptr_null(l);
}
END_TEST

START_TEST(test_l_init_exist)
{
	Lexer *l;

	l = l_init("files/test_lex.txt");
	ck_assert_ptr_nonnull(l);
	ck_assert_str_eq("files/test_lex.txt", l->filename);
	ck_assert_ptr_eq(l->pos, l->data);
	ck_assert_ptr_null(l->err);
	ck_assert_ptr_nonnull(l->pos);
	ck_assert_ptr_nonnull(l->data);

	l_free(l);
}
END_TEST

START_TEST(test_lex_lparen)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("files/test_lex_lparen.txt");

	le = lex(l);
	ck_assert(le->type == LE_LPAREN);
	ck_assert_str_eq(le->lexeme, "(");

	free(le);
	free(le->lexeme);
	l_free(l);
}
END_TEST

START_TEST(test_lex_number)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("files/test_lex_number.txt");

	le = lex(l);
	ck_assert(le->type == LE_NUMBER);
	ck_assert_str_eq(le->lexeme, "5.44");

	free(le);
	free(le->lexeme);
	l_free(l);
}
END_TEST

START_TEST(test_lex_operator)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("files/test_lex_operator.txt");

	le = lex(l);
	ck_assert(le->type == LE_OPERATOR);
	ck_assert_str_eq(le->lexeme, "*");

	free(le);
	free(le->lexeme);
	l_free(l);
}
END_TEST

START_TEST(test_lex_rparen)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("files/test_lex_rparen.txt");

	le = lex(l);
	ck_assert(le->type == LE_RPAREN);
	ck_assert_str_eq(le->lexeme, ")");

	free(le);
	free(le->lexeme);
	l_free(l);
}
END_TEST

START_TEST(test_lex_symbol)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("files/test_lex_symbol.txt");

	le = lex(l);
	ck_assert(le->type == LE_SYMBOL);
	ck_assert_str_eq(le->lexeme, "sin");

	free(le);
	free(le->lexeme);
	l_free(l);
}
END_TEST

START_TEST(test_lex_full)
{
	Lexeme *le;
	Lexer *l;

	l = l_init("files/test_lex_full.txt");

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
	ck_assert_str_eq(le->lexeme, "3");
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
	ck_assert(le->type == LE_NUMBER);
	ck_assert_str_eq(le->lexeme, "5.2");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_OPERATOR);
	ck_assert_str_eq(le->lexeme, "*");
	free(le);
	free(le->lexeme);

	le = lex(l);
	ck_assert(le->type == LE_SYMBOL);
	ck_assert_str_eq(le->lexeme, "y");
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

	l_free(l);
}
END_TEST

START_TEST(test_parse_empty)
{
	Parser *p;

	p = p_init("files/test_parse_empty.txt");

	ck_assert(parse(p) == 0);
	ck_assert_ptr_null(p->ast);
	p_free(p);
}
END_TEST

START_TEST(test_parse_malformed_expression)
{
	Parser *p;
	p = p_init("files/test_parse_malformed_expression.txt");

	ck_assert_msg(parse(p) < 0, "Parser should fail but it doesn't");
	ck_assert_str_eq(p->err, "files/test_parse_malformed_expression.txt: malformed expression\n");
	p_free(p);
}
END_TEST

START_TEST(test_parse_non_parenthesized)
{
	Parser *p;
	p = p_init("files/test_parse_non_parenthesized.txt");
	ck_assert_msg(parse(p) == 0, p->err);

	ck_assert(p->ast->sym->content->op == '/');
	ck_assert_str_eq(p->ast->right->sym->content->func, "sin");
	ck_assert(num_equal(p->ast->right->right->sym, 2));
	ck_assert_ptr_null(p->ast->right->left);
	ck_assert(p->ast->left->sym->content->op == '+');
	ck_assert(num_equal(p->ast->left->right->sym, 4));
	ck_assert(p->ast->left->left->sym->content->op == '*');
	ck_assert(p->ast->left->left->right->sym->content->var == 'x');
	ck_assert(num_equal(p->ast->left->left->left->sym, 2));

	p_free(p);
}
END_TEST

START_TEST(test_parse_parenthesized)
{
	Parser *p;

	p = p_init("files/test_parse_parenthesized.txt");
	ck_assert_msg(parse(p) == 0, p->err);

	ck_assert(p->ast->sym->content->op == '/');
	ck_assert_str_eq(p->ast->right->sym->content->func, "sin");
	ck_assert(num_equal(p->ast->right->right->sym, 2));
	ck_assert_ptr_null(p->ast->right->left);
	ck_assert(p->ast->left->sym->content->op == '+');
	ck_assert(num_equal(p->ast->left->right->sym, 4));
	ck_assert(p->ast->left->left->sym->content->op == '*');
	ck_assert(p->ast->left->left->right->sym->content->var == 'x');
	ck_assert(num_equal(p->ast->left->left->left->sym, 2));

	p_free(p);
}
END_TEST

START_TEST(test_parse_unbalanced_left_parenthesis)
{
	Parser *p;
	p = p_init("files/test_parse_unbalanced_left_parenthesis.txt");

	ck_assert_msg(parse(p) < 0, "Parser should fail but it doesn't");
	ck_assert_str_eq(p->err, "files/test_parse_unbalanced_left_parenthesis.txt: unbalanced parenthesis\n");
	p_free(p);
}
END_TEST

START_TEST(test_parse_unbalanced_right_parenthesis)
{
	Parser *p;
	p = p_init("files/test_parse_unbalanced_right_parenthesis.txt");

	ck_assert_msg(parse(p) < 0, "Parser should fail but it doesn't");
	ck_assert_str_eq(p->err, "files/test_parse_unbalanced_right_parenthesis.txt: unbalanced parenthesis\n");
	p_free(p);
}
END_TEST

/* START_TEST(test_parse_function_application) */
/* { */
/* } */
/* END_TEST */

/* START_TEST(test_parse_function_no_parenthesis) */
/* { */
/* } */
/* END_TEST */

Suite*
parse_suite(void)
{
	Suite *s;
	TCase *tc_lex, *tc_parse;

	s = suite_create("parse");

	tc_lex = tcase_create("lex");
	tc_parse = tcase_create("parse");

	tcase_add_test(tc_lex, test_l_init_non_exist);
	tcase_add_test(tc_lex, test_l_init_exist);
	tcase_add_test(tc_lex, test_lex_full);
	tcase_add_test(tc_lex, test_lex_lparen);
	tcase_add_test(tc_lex, test_lex_number);
	tcase_add_test(tc_lex, test_lex_operator);
	tcase_add_test(tc_lex, test_lex_rparen);
	tcase_add_test(tc_lex, test_lex_symbol);

	tcase_add_test(tc_parse, test_parse_empty);
	tcase_add_test(tc_parse, test_parse_malformed_expression);
	tcase_add_test(tc_parse, test_parse_non_parenthesized);
	tcase_add_test(tc_parse, test_parse_parenthesized);
	tcase_add_test(tc_parse, test_parse_unbalanced_left_parenthesis);
	tcase_add_test(tc_parse, test_parse_unbalanced_right_parenthesis);

	suite_add_tcase(s, tc_lex);
	suite_add_tcase(s, tc_parse);

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
