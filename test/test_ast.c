/*
 * Copyright ©️ 2022 Mario Forzanini <mf@marioforzanini.com>
 *
 * This file is part of dwrt.
 *
 * Dwrt is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Dwrt is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dwrt. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "../dat.h"
#include "../fns.h"

/* Test allocations */

START_TEST(test_ast_alloc)
{
	Node *node;

	node = ast_alloc(func_alloc("sin"));
	ck_assert_ptr_null(node->left);
	ck_assert_ptr_null(node->right);
	ck_assert_ptr_null(node->parent);

	ast_free(node);
}
END_TEST

START_TEST(test_func_alloc)
{
	Symbol *sym;

	sym = func_alloc("sin");
	ck_assert(sym->type == S_FUNC);
	ck_assert_str_eq(sym->content->func, "sin");

	symbol_free(sym);
}
END_TEST

START_TEST(test_lparen_alloc)
{
	Symbol *sym;

	sym = lparen_alloc();
	ck_assert(sym->type == S_LPAREN);
	ck_assert_str_eq(sym->content->func, "(");

	symbol_free(sym);
}
END_TEST

START_TEST(test_num_alloc)
{
	Symbol *sym;

	sym = num_alloc(5);
	ck_assert(sym->type == S_NUM);
	ck_assert_double_eq(sym->content->num, 5);

	symbol_free(sym);
}
END_TEST

START_TEST(test_operator_alloc)
{
	Symbol *sym;

	sym = operator_alloc('+');
	ck_assert(sym->type == S_OP);
	ck_assert(sym->content->op == '+');

	symbol_free(sym);
}
END_TEST

START_TEST(test_rparen_alloc)
{
	Symbol *sym;

	sym = rparen_alloc();
	ck_assert(sym->type == S_RPAREN);
	ck_assert_str_eq(sym->content->func, ")");

	symbol_free(sym);
}
END_TEST

START_TEST(test_var_alloc)
{
	Symbol *sym;

	sym = var_alloc('x');
	ck_assert(sym->type == S_VAR);
	ck_assert(sym->content->var == 'x');

	symbol_free(sym);
}
END_TEST

/* Test ast manipulation functions */

START_TEST(test_ast_copy_null)
{
	Node *dest;

	dest = ast_copy(NULL);
	ck_assert_ptr_null(dest);
}
END_TEST

START_TEST(test_ast_copy_shallow)
{
	Node *dest, *src;

	src = ast_alloc(num_alloc(5));

	dest = ast_copy(src);
	ck_assert(is_num(dest->sym));
	ck_assert(num_equal(dest->sym, 5));
	ck_assert(dest != src);
	ck_assert_ptr_null(dest->right);
	ck_assert_ptr_null(dest->left);

	ast_free(dest);
	ast_free(src);
}
END_TEST

START_TEST(test_ast_copy_deep)
{
	Node *dest, *src;

	src = ast_alloc(operator_alloc('+'));
	src->left = ast_alloc(num_alloc(5));
	src->right = ast_alloc(func_alloc("sin"));
	src->right->right = ast_alloc(num_alloc(6));

	dest = ast_copy(src);
	ck_assert(is_operator(dest->sym));
	ck_assert(dest->sym->content->op == '+');
	ck_assert(dest != src);
	ck_assert_ptr_nonnull(dest->right);
	ck_assert_ptr_nonnull(dest->left);

	ck_assert(dest->right != src->right);
	ck_assert(dest->left != src->left);

	ck_assert(is_num(dest->left->sym));
	ck_assert(num_equal(dest->left->sym, 5));

	ck_assert(is_function(dest->right->sym));
	ck_assert_str_eq(dest->right->sym->content->func, "sin");

	ck_assert_ptr_null(dest->right->left);
	ck_assert(num_equal(dest->right->right->sym, 6));

	ast_free(dest);
	ast_free(src);
}
END_TEST

START_TEST(test_ast_insert_null)
{
	Node *node;

	node = ast_alloc(num_alloc(5));
	ast_insert(node, NULL);
	ck_assert_ptr_null(node->left);
	ck_assert_ptr_null(node->right);

	ast_free(node);
}
END_TEST

START_TEST(test_ast_insert_in_null)
{
	ast_insert(NULL, NULL);
}
END_TEST

START_TEST(test_ast_insert)
{
	Node *node;

	node = ast_alloc(operator_alloc('*'));

	ast_insert(node, ast_alloc(num_alloc(42)));
	ck_assert_ptr_nonnull(node->right);
	ck_assert(num_equal(node->right->sym, 42));

	ast_insert(node, ast_alloc(num_alloc(55)));
	ck_assert_ptr_nonnull(node->left);
	ck_assert(num_equal(node->left->sym, 55));

	ast_free(node);
}
END_TEST

/* Test predicates */

START_TEST(test_is_function)
{
	Symbol *sym;

	sym = func_alloc("sin");
	ck_assert(is_function(sym));

	symbol_free(sym);
}
END_TEST

START_TEST(test_is_lparen)
{
	Symbol *sym;

	sym = lparen_alloc();
	ck_assert(is_lparen(sym));

	symbol_free(sym);
}
END_TEST

START_TEST(test_is_operator)
{
	Symbol *sym;

	sym = operator_alloc('+');
	ck_assert(is_operator(sym));

	symbol_free(sym);
}
END_TEST

START_TEST(test_is_num)
{
	Symbol *sym;

	sym = num_alloc(44);
	ck_assert(is_num(sym));

	symbol_free(sym);
}
END_TEST

START_TEST(test_num_equal)
{
	Symbol *sym;

	sym = num_alloc(44);
	ck_assert(is_num(sym));
	ck_assert(num_equal(sym, 44));

	symbol_free(sym);
}
END_TEST

Suite*
ast_suite(void)
{
	Suite *s;
	TCase *tc_alloc, *tc_ast, *tc_predicates;

	s = suite_create("ast");

	tc_alloc = tcase_create("alloc");
	tc_ast = tcase_create("ast");
	tc_predicates = tcase_create("predicates");

	tcase_add_test(tc_alloc, test_ast_alloc);
	tcase_add_test(tc_alloc, test_func_alloc);
	tcase_add_test(tc_alloc, test_lparen_alloc);
	tcase_add_test(tc_alloc, test_num_alloc);
	tcase_add_test(tc_alloc, test_operator_alloc);
	tcase_add_test(tc_alloc, test_rparen_alloc);
	tcase_add_test(tc_alloc, test_var_alloc);

	tcase_add_test(tc_ast, test_ast_copy_null);
	tcase_add_test(tc_ast, test_ast_copy_shallow);
	tcase_add_test(tc_ast, test_ast_copy_deep);
	tcase_add_test(tc_ast, test_ast_insert_null);
	tcase_add_test(tc_ast, test_ast_insert_in_null);
	tcase_add_test(tc_ast, test_ast_insert);

	tcase_add_test(tc_predicates, test_is_function);
	tcase_add_test(tc_predicates, test_is_lparen);
	tcase_add_test(tc_predicates, test_is_operator);
	tcase_add_test(tc_predicates, test_is_num);
	tcase_add_test(tc_predicates, test_num_equal);

	suite_add_tcase(s, tc_alloc);
	suite_add_tcase(s, tc_ast);
	suite_add_tcase(s, tc_predicates);

	return s;
}

int
main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = ast_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
