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

#include "../dwrt.c"

START_TEST(test_dwrt_op_expt_var_to_num)
{
	Node *expt, *diff;

	expt = ast_expt(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(7)));
	diff = ast_dwrt(expt, 'x');
	ck_assert_ptr_nonnull(diff);

	ck_assert_int_eq(diff->sym->type, S_OP);
	ck_assert(diff->sym->content->op == '*');

	ck_assert(is_num(diff->left->sym));
	ck_assert_double_eq(diff->left->sym->content->num, 7);

	ck_assert(is_operator(diff->right->sym));
	ck_assert(diff->right->sym->content->op == '^');

	ck_assert(is_same_var(diff->right->left->sym, 'x'));

	ck_assert(is_num(diff->right->right->sym));
	ck_assert_double_eq(diff->right->right->sym->content->num, 6);

	ast_free(expt);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_op_expt_two_num)
{
	Node *expt, *diff;

	expt = ast_alloc(operator_alloc('^'));
	ast_insert(expt, ast_alloc(num_alloc(5)));
	ast_insert(expt, ast_alloc(num_alloc(2)));

	diff = ast_dwrt(expt, 'x');
	ck_assert_ptr_nonnull(diff);
	ck_assert(is_num(diff->sym));
	ck_assert_double_eq(diff->sym->content->num, 0);

	ast_free(expt);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_op_expt_var_to_func)
{
	Node *expt, *diff;

	expt = ast_expt(ast_alloc(var_alloc('x')), ast_alloc(var_alloc('x')));
	diff = ast_dwrt(expt, 'x');

	ck_assert_ptr_nonnull(diff);

	ast_free(expt);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_op_frac)
{
	Node *ast, *diff;

	ast = ast_frac(ast_alloc(num_alloc(1)), ast_alloc(var_alloc('x')));
	diff = ast_dwrt(ast, 'x');
	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_OP);
	ck_assert(diff->sym->content->op == '/');

	ck_assert(diff->left->sym->type == S_NUM);
	ck_assert_double_eq(diff->left->sym->content->num, -1);
	ck_assert(ast->left != diff->left);

	ck_assert(diff->right->sym->type == S_OP);
	ck_assert(diff->right->sym->content->op == '^');

	ck_assert(is_same_var(diff->right->left->sym, 'x'));
	ck_assert(num_equal(diff->right->right->sym, 2));

	ast_free(diff);
	ast_free(ast);
}
END_TEST

START_TEST(test_dwrt_op_mul)
{
	Node *ast, *diff;

	ast = ast_mul(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(5)));
	diff = ast_dwrt(ast, 'x');

	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_NUM);
	ck_assert_double_eq(diff->sym->content->num, 5);

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_op_sub)
{
	Node *ast, *diff;

	ast = ast_sub(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(5)));
	diff = ast_dwrt(ast, 'x');
	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_NUM);
	ck_assert_double_eq(diff->sym->content->num, 1);

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_op_sum)
{
	Node *ast, *diff;

	ast = ast_sub(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(5)));
	diff = ast_dwrt(ast, 'x');
	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_NUM);
	ck_assert_double_eq(diff->sym->content->num, 1);

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_func_cos)
{
	Node *ast, *diff;

	ast = ast_cos(ast_alloc(var_alloc('x')));
	diff = ast_dwrt(ast, 'x');

	ck_assert_ptr_nonnull(diff);

	ck_assert(diff->sym->type == S_OP);
	ck_assert(diff->sym->content->op == '*');

	ck_assert(diff->left->sym->type == S_NUM);
	ck_assert_double_eq(diff->left->sym->content->num, -1);

	ck_assert(diff->right->sym->type == S_FUNC);
	ck_assert_str_eq(diff->right->sym->content->func, "sin");

	ast_free(ast);
	ast_free(diff);
}
END_TEST


START_TEST(test_dwrt_func_cosh)
{
	Node *ast, *diff;

	ast = ast_cosh(ast_alloc(var_alloc('x')));
	diff = ast_dwrt(ast, 'x');

	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_FUNC);
	ck_assert_str_eq(diff->sym->content->func, "sinh");

	ck_assert(diff->right->sym->type == S_VAR);
	ck_assert(diff->right->sym->content->var == 'x');

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_func_exp)
{
	Node *ast, *diff;

	ast = ast_exp(ast_alloc(var_alloc('x')));
	diff = ast_dwrt(ast, 'x');

	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_FUNC);
	ck_assert_str_eq(diff->sym->content->func, "exp");

	ck_assert(diff->right->sym->type == S_VAR);
	ck_assert(diff->right->sym->content->var == 'x');

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_func_log)
{
	Node *ast, *diff;

	ast = ast_log(ast_alloc(var_alloc('x')));
	diff = ast_dwrt(ast, 'x');

	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_OP);
	ck_assert(diff->sym->content->op == '/');

	ck_assert(is_num(diff->left->sym));
	ck_assert(num_equal(diff->left->sym, 1));

	ck_assert(diff->right->sym->type == S_VAR);
	ck_assert(diff->right->sym->content->var == 'x');

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_func_sin)
{
	Node *ast, *diff;

	ast = ast_sin(ast_alloc(var_alloc('x')));
	diff = ast_dwrt(ast, 'x');

	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_FUNC);
	ck_assert_str_eq(diff->sym->content->func, "cos");

	ck_assert(diff->right->sym->type == S_VAR);
	ck_assert(diff->right->sym->content->var == 'x');

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_func_sinh)
{
	Node *ast, *diff;

	ast = ast_sinh(ast_alloc(var_alloc('x')));
	diff = ast_dwrt(ast, 'x');

	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_FUNC);
	ck_assert_str_eq(diff->sym->content->func, "cosh");

	ck_assert(diff->right->sym->type == S_VAR);
	ck_assert(diff->right->sym->content->var == 'x');

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_func_tan)
{
	Node *ast, *diff;

	ast = ast_tan(ast_alloc(var_alloc('x')));
	diff = ast_dwrt(ast, 'x');

	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_OP);
	ck_assert(diff->sym->content->op == '+');

	ck_assert(diff->left->sym->type == S_NUM);
	ck_assert_double_eq(diff->left->sym->content->num, 1);

	ck_assert(diff->right->sym->type == S_OP);
	ck_assert(diff->right->sym->content->op == '^');

	ck_assert(diff->right->left->sym->type == S_FUNC);
	ck_assert_str_eq(diff->right->left->sym->content->func, "tan");

	ck_assert(diff->right->left->right->sym->type == S_VAR);
	ck_assert(diff->right->left->right->sym->content->op == 'x');

	ck_assert(num_equal(diff->right->right->sym, 2));

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_dwrt_func_tanh)
{
	Node *ast, *diff;

	ast = ast_tanh(ast_alloc(var_alloc('x')));
	diff = ast_dwrt(ast, 'x');

	ck_assert_ptr_nonnull(diff);
	ck_assert(diff->sym->type == S_OP);
	ck_assert(diff->sym->content->op == '-');

	ck_assert(diff->right->sym->type == S_NUM);
	ck_assert_double_eq(diff->right->sym->content->num, 1);

	ck_assert(diff->left->sym->type == S_OP);
	ck_assert(diff->left->sym->content->op == '^');

	ck_assert(num_equal(diff->left->right->sym, 2));

	ck_assert(diff->left->left->sym->type == S_FUNC);
	ck_assert_str_eq(diff->left->left->sym->content->func, "tanh");

	ck_assert(diff->left->left->right->sym->type == S_VAR);
	ck_assert(diff->left->left->right->sym->content->op == 'x');

	ast_free(ast);
	ast_free(diff);
}
END_TEST

START_TEST(test_ast_cos)
{
	Node *ast;

	ast = ast_cos(ast_alloc(var_alloc('y')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->left);
	ck_assert_ptr_nonnull(ast->right);

	ck_assert_str_eq(ast->sym->content->func, "cos");
	ck_assert(ast->right->sym->type == S_VAR);
	ck_assert(ast->right->sym->content->var == 'y');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_cos_null)
{
	Node *ast;

	ast = ast_cos(NULL);
	ck_assert_ptr_null(ast);

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_cosh)
{
	Node *ast;

	ast = ast_cosh(ast_alloc(var_alloc('y')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->left);
	ck_assert_ptr_nonnull(ast->right);

	ck_assert_str_eq(ast->sym->content->func, "cosh");
	ck_assert(ast->right->sym->type == S_VAR);
	ck_assert(ast->right->sym->content->var == 'y');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_cosh_null)
{
	Node *ast;

	ast = ast_cosh(NULL);
	ck_assert_ptr_null(ast);
}
END_TEST

START_TEST(test_ast_exp)
{
	Node *ast;

	ast = ast_exp(ast_alloc(var_alloc('y')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->left);
	ck_assert_ptr_nonnull(ast->right);

	ck_assert_str_eq(ast->sym->content->func, "exp");
	ck_assert(ast->right->sym->type == S_VAR);
	ck_assert(ast->right->sym->content->var == 'y');

	ast_free(ast);

}
END_TEST

START_TEST(test_ast_exp_null)
{
	Node *ast;

	ast = ast_exp(NULL);
	ck_assert_ptr_null(ast);
}
END_TEST

START_TEST(test_ast_expt_left_is_one)
{
	Node *expt;

	expt = ast_expt(ast_alloc(num_alloc(1)), ast_alloc(var_alloc('x')));
	ck_assert_ptr_nonnull(expt);
	ck_assert(is_num(expt->sym));
	ck_assert_double_eq(expt->sym->content->num, 1);

	ast_free(expt);
}
END_TEST

START_TEST(test_ast_expt_right_is_zero)
{
	Node *expt;

	expt = ast_expt(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(0)));
	ck_assert_ptr_nonnull(expt);
	ck_assert(is_num(expt->sym));
	ck_assert_double_eq(expt->sym->content->num, 1);

	ast_free(expt);
}
END_TEST

START_TEST(test_ast_expt_two_num)
{
	Node *expt;

	expt = ast_expt(ast_alloc(num_alloc(5)), ast_alloc(num_alloc(2)));
	ck_assert_ptr_nonnull(expt);
	ck_assert(is_num(expt->sym));
	ck_assert_double_eq(expt->sym->content->num, 25);

	ast_free(expt);
}
END_TEST

START_TEST(test_ast_expt)
{
	Node *expt;

	expt = ast_expt(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(5)));
	ck_assert_ptr_nonnull(expt);
	ck_assert(is_operator(expt->sym));
	ck_assert(expt->sym->content->op == '^');
	ck_assert(is_same_var(expt->left->sym, 'x'));
	ck_assert(is_num(expt->right->sym));
	ck_assert_double_eq(expt->right->sym->content->num, 5);

	ast_free(expt);
}
END_TEST

START_TEST(test_ast_log)
{
	Node *ast;

	ast = ast_log(ast_alloc(var_alloc('y')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->left);
	ck_assert_ptr_nonnull(ast->right);

	ck_assert_str_eq(ast->sym->content->func, "log");
	ck_assert(ast->right->sym->type == S_VAR);
	ck_assert(ast->right->sym->content->var == 'y');

	ast_free(ast);

}
END_TEST

START_TEST(test_ast_log_null)
{
	Node *ast;

	ast = ast_log(NULL);
	ck_assert_ptr_null(ast);
}
END_TEST

START_TEST(test_ast_frac_right_is_one)
{
	Node *ast;

	ast = ast_frac(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(1)));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_VAR);
	ck_assert(ast->sym->content->var == 'x');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_frac_right_is_zero)
{
	Node *ast;

	ast = ast_frac(ast_alloc(var_alloc('x')) , ast_alloc(num_alloc(0)));

	ck_assert_ptr_null(ast);
}
END_TEST


START_TEST(test_ast_frac_left_is_zero)
{
	Node *ast;

	ast = ast_frac(ast_alloc(num_alloc(0)) , ast_alloc(var_alloc('x')));

	ck_assert_ptr_nonnull(ast);
	ck_assert(is_num(ast->sym));
	ck_assert_double_eq(ast->sym->content->num, 0);

	ast_free(ast);
}
END_TEST


START_TEST(test_ast_frac_two_num)
{
	Node *ast;

	ast = ast_frac(ast_alloc(num_alloc(10)), ast_alloc(num_alloc(2)));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert_double_eq(ast->sym->content->num, 10. / 2.);
	ast_free(ast);
}
END_TEST

START_TEST(test_ast_frac)
{
	Node *ast;

	ast = ast_frac(ast_alloc(num_alloc(10)), ast_alloc(var_alloc('z')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_nonnull(ast->right);
	ck_assert_ptr_nonnull(ast->left);

	ck_assert(ast->sym->content->op == '/');
	ck_assert_double_eq(ast->left->sym->content->num, 10);
	ck_assert(ast->right->sym->content->var == 'z');

	ast_free(ast);
}
END_TEST


START_TEST(test_ast_mul_left_is_one)
{
	Node *ast;

	ast = ast_mul(ast_alloc(num_alloc(1)), ast_alloc(var_alloc('x')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_VAR);
	ck_assert(ast->sym->content->var == 'x');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_mul_left_is_zero)
{
	Node *ast;

	ast = ast_mul(ast_alloc(num_alloc(0)), ast_alloc(var_alloc('x')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_NUM);
	ck_assert_double_eq(ast->sym->content->num, 0);

	ast_free(ast);
}
END_TEST


START_TEST(test_ast_mul_right_is_one)
{
	Node *ast;

	ast = ast_mul(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(1)));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_VAR);
	ck_assert(ast->sym->content->var == 'x');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_mul_right_is_zero)
{
	Node *ast;

	ast = ast_mul(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(0)));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_NUM);
	ck_assert_double_eq(ast->sym->content->num, 0);

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_mul_two_num)
{
	Node *ast;

	ast = ast_mul(ast_alloc(num_alloc(5)), ast_alloc(num_alloc(7)));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_NUM);
	ck_assert_double_eq(ast->sym->content->num, 35);

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_mul)
{
	Node *ast;

	ast = ast_mul(ast_alloc(var_alloc('x')), ast_alloc(var_alloc('y')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_nonnull(ast->right);
	ck_assert_ptr_nonnull(ast->left);

	ck_assert(is_operator(ast->sym));
	ck_assert(ast->sym->content->op == '*');
	ck_assert(ast->left->sym->content->var == 'x');
	ck_assert(ast->right->sym->content->var == 'y');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sin)
{
	Node *ast;

	ast = ast_sin(ast_alloc(var_alloc('y')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->left);
	ck_assert_ptr_nonnull(ast->right);

	ck_assert_str_eq(ast->sym->content->func, "sin");
	ck_assert(ast->right->sym->type == S_VAR);
	ck_assert(ast->right->sym->content->var == 'y');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sin_null)
{
	Node *ast;

	ast = ast_sin(NULL);
	ck_assert_ptr_null(ast);
}
END_TEST

START_TEST(test_ast_sinh)
{
	Node *ast;

	ast = ast_sinh(ast_alloc(var_alloc('y')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->left);
	ck_assert_ptr_nonnull(ast->right);

	ck_assert_str_eq(ast->sym->content->func, "sinh");
	ck_assert(ast->right->sym->type == S_VAR);
	ck_assert(ast->right->sym->content->var == 'y');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sinh_null)
{
	Node *ast;

	ast = ast_sinh(NULL);
	ck_assert_ptr_null(ast);
}
END_TEST

START_TEST(test_ast_sub_left_is_zero)
{
	Node *ast;

	ast = ast_sub(ast_alloc(num_alloc(0)), ast_alloc(var_alloc('x')));
	ck_assert_ptr_nonnull(ast);

	ck_assert(is_operator(ast->sym));
	ck_assert(ast->sym->content->op == '*');

	ck_assert(is_num(ast->left->sym));
	ck_assert(num_equal(ast->left->sym, -1));

	ck_assert(ast->right->sym->type == S_VAR);
	ck_assert(ast->right->sym->content->var == 'x');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sub_right_is_zero)
{
	Node *ast;

	ast = ast_sub(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(0)));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_VAR);
	ck_assert(ast->sym->content->var == 'x');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sub_two_num)
{
	Node *ast;

	ast = ast_sub(ast_alloc(num_alloc(7)), ast_alloc(num_alloc(5)));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_NUM);
	ck_assert_double_eq(ast->sym->content->num, 2);

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sub)
{
	Node *ast;

	ast = ast_sub(ast_alloc(var_alloc('x')), ast_alloc(var_alloc('y')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_nonnull(ast->right);
	ck_assert_ptr_nonnull(ast->left);

	ck_assert(is_operator(ast->sym));
	ck_assert(ast->sym->content->op == '-');
	ck_assert(ast->left->sym->content->var == 'x');
	ck_assert(ast->right->sym->content->var == 'y');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sum_left_is_zero)
{
	Node *ast;

	ast = ast_sum(ast_alloc(num_alloc(0)), ast_alloc(var_alloc('x')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_VAR);
	ck_assert(ast->sym->content->var == 'x');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sum_right_is_zero)
{
	Node *ast;

	ast = ast_sum(ast_alloc(var_alloc('x')), ast_alloc(num_alloc(0)));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_VAR);
	ck_assert(ast->sym->content->var == 'x');

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sum_two_num)
{
	Node *ast;

	ast = ast_sum(ast_alloc(num_alloc(5)), ast_alloc(num_alloc(7)));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_null(ast->right);
	ck_assert_ptr_null(ast->left);

	ck_assert(ast->sym->type == S_NUM);
	ck_assert_double_eq(ast->sym->content->num, 12);

	ast_free(ast);
}
END_TEST

START_TEST(test_ast_sum)
{
	Node *ast;

	ast = ast_sum(ast_alloc(var_alloc('x')), ast_alloc(var_alloc('y')));
	ck_assert_ptr_nonnull(ast);
	ck_assert_ptr_nonnull(ast->right);
	ck_assert_ptr_nonnull(ast->left);

	ck_assert(is_operator(ast->sym));
	ck_assert(ast->sym->content->op == '+');
	ck_assert(ast->left->sym->content->var == 'x');
	ck_assert(ast->right->sym->content->var == 'y');

	ast_free(ast);
}
END_TEST

START_TEST(test_is_same_var_fail)
{
	Symbol *sym;

	sym = var_alloc('x');

	ck_assert(! is_same_var(sym, 'y'));

	symbol_free(sym);
}
END_TEST

START_TEST(test_is_same_var_not_var)
{
	Symbol *sym;

	sym = num_alloc(5);
	ck_assert(! is_same_var(sym, 'x'));

	symbol_free(sym);
}
END_TEST

START_TEST(test_is_same_var)
{
	Symbol *sym;

	sym = var_alloc('x');

	ck_assert(is_same_var(sym, 'x'));
	symbol_free(sym);
}
END_TEST

Suite*
dwrt_suite()
{
	Suite *s;
	TCase *tc_dwrt, *tc_expt, *tc_frac, *tc_func, *tc_mul, *tc_sub, *tc_sum, *tc_var;

	s = suite_create("dwrt");

	tc_dwrt = tcase_create("dwrt");
	tc_expt = tcase_create("expt");
	tc_frac = tcase_create("frac");
	tc_func = tcase_create("func");
	tc_mul = tcase_create("mul");
	tc_sub = tcase_create("sub");
	tc_sum = tcase_create("sum");
	tc_var = tcase_create("var");

	tcase_add_test(tc_dwrt, test_dwrt_func_cos);
	tcase_add_test(tc_dwrt, test_dwrt_func_cosh);
	tcase_add_test(tc_dwrt, test_dwrt_func_exp);
	tcase_add_test(tc_dwrt, test_dwrt_func_log);
	tcase_add_test(tc_dwrt, test_dwrt_func_sin);
	tcase_add_test(tc_dwrt, test_dwrt_func_sinh);
	tcase_add_test(tc_dwrt, test_dwrt_func_tan);
	tcase_add_test(tc_dwrt, test_dwrt_func_tanh);
	tcase_add_test(tc_dwrt, test_dwrt_op_sum);
	tcase_add_test(tc_dwrt, test_dwrt_op_sub);
	tcase_add_test(tc_dwrt, test_dwrt_op_mul);
	tcase_add_test(tc_dwrt, test_dwrt_op_frac);
	tcase_add_test(tc_dwrt, test_dwrt_op_expt_two_num);
	tcase_add_test(tc_dwrt, test_dwrt_op_expt_var_to_num);
	tcase_add_test(tc_dwrt, test_dwrt_op_expt_var_to_func);

	tcase_add_test(tc_expt, test_ast_expt_two_num);
	tcase_add_test(tc_expt, test_ast_expt_left_is_one);
	tcase_add_test(tc_expt, test_ast_expt_right_is_zero);
	tcase_add_test(tc_expt, test_ast_expt);

	tcase_add_test(tc_frac, test_ast_frac_two_num);
	tcase_add_test(tc_frac, test_ast_frac_left_is_zero);
	tcase_add_test(tc_frac, test_ast_frac_right_is_one);
	tcase_add_test(tc_frac, test_ast_frac_right_is_zero);
	tcase_add_test(tc_frac, test_ast_frac);

	tcase_add_test(tc_func, test_ast_cos);
	tcase_add_test(tc_func, test_ast_cos_null);
	tcase_add_test(tc_func, test_ast_cosh);
	tcase_add_test(tc_func, test_ast_cosh_null);
	tcase_add_test(tc_func, test_ast_exp);
	tcase_add_test(tc_func, test_ast_exp_null);
	tcase_add_test(tc_func, test_ast_log);
	tcase_add_test(tc_func, test_ast_log_null);
	tcase_add_test(tc_func, test_ast_sin);
	tcase_add_test(tc_func, test_ast_sin_null);
	tcase_add_test(tc_func, test_ast_sinh);
	tcase_add_test(tc_func, test_ast_sinh_null);

	tcase_add_test(tc_mul, test_ast_mul_left_is_one);
	tcase_add_test(tc_mul, test_ast_mul_left_is_zero);
	tcase_add_test(tc_mul, test_ast_mul_right_is_one);
	tcase_add_test(tc_mul, test_ast_mul_right_is_zero);
	tcase_add_test(tc_mul, test_ast_mul_two_num);
	tcase_add_test(tc_mul, test_ast_mul);

	tcase_add_test(tc_sub, test_ast_sub_left_is_zero);
	tcase_add_test(tc_sub, test_ast_sub_right_is_zero);
	tcase_add_test(tc_sub, test_ast_sub_two_num);
	tcase_add_test(tc_sub, test_ast_sub);

	tcase_add_test(tc_sum, test_ast_sum_left_is_zero);
	tcase_add_test(tc_sum, test_ast_sum_right_is_zero);
	tcase_add_test(tc_sum, test_ast_sum_two_num);
	tcase_add_test(tc_sum, test_ast_sum);

	tcase_add_test(tc_var, test_is_same_var_fail);
	tcase_add_test(tc_var, test_is_same_var_not_var);
	tcase_add_test(tc_var, test_is_same_var);

	suite_add_tcase(s, tc_dwrt);
	suite_add_tcase(s, tc_expt);
	suite_add_tcase(s, tc_frac);
	suite_add_tcase(s, tc_func);
	suite_add_tcase(s, tc_mul);
	suite_add_tcase(s, tc_sum);
	suite_add_tcase(s, tc_sub);
	suite_add_tcase(s, tc_var);

	return s;
}

int
main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = dwrt_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
