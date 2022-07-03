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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static Node*	ast_cos(Node*);
static Node*	ast_cosh(Node*);
static Node*	ast_dwrt_func(Node*, char);
static Node*	ast_dwrt_op(Node*, char);
static Node*	ast_exp(Node*);
static Node*	ast_expt(Node*, Node*);
static Node*	ast_frac(Node*, Node*);
static Node*	ast_log(Node*);
static Node*	ast_mul(Node*, Node*);
static Node*	ast_sin(Node*);
static Node*	ast_sinh(Node*);
static Node*	ast_sum(Node*, Node*);
static Node*	ast_sub(Node*, Node*);
static Node*	ast_tan(Node*);
static Node*	ast_tanh(Node*);
static int	is_same_var(Symbol*, char);

static Node*
ast_cos(Node *x)
{
	Node *cos;
	char *f;
	if(x == NULL)
		return NULL;

	f = ecalloc(4, sizeof(char));
	f = strcpy(f, "cos");
	cos = ast_alloc(func_alloc(f));
	ast_insert(cos, x);
	return cos;
}

static Node*
ast_cosh(Node *x)
{
	Node *cosh;
	char *f;

	if(x == NULL)
		return NULL;

	f = ecalloc(5, sizeof(char));
	f = strcpy(f, "cosh");
	cosh = ast_alloc(func_alloc(f));
	ast_insert(cosh, x);
	return cosh;
}

/*
 * TODO: symplify numerical expressions
 */
Node*
ast_dwrt(Node *ast, char var)
{
	if(ast == NULL)
		return NULL;
	switch(ast->sym->type) {
	case S_VAR:
		if(is_same_var(ast->sym, var))
			return ast_alloc(num_alloc(1));
		else
			return ast_alloc(num_alloc(0));
		break;
	case S_NUM:
		return ast_alloc(num_alloc(0));
		break;
	case S_OP:
		return ast_dwrt_op(ast, var);
		break;
	case S_FUNC:
		return ast_dwrt_func(ast, var);
	default:
		break;
	}
	return NULL;
}

static Node*
ast_dwrt_func(Node *ast, char var)
{
	Node *arg;

	arg = ast->right;
	if(strcmp(ast->sym->content->func, "cos") == 0) {
		return ast_mul(ast_dwrt(arg, var),
		 ast_mul(ast_alloc(num_alloc(-1)), ast_sin(ast_copy(arg))));
	} else if(strcmp(ast->sym->content->func, "cosh") == 0) {
		return ast_mul(ast_dwrt(arg, var), ast_sinh(ast_copy(arg)));
	} else if(strcmp(ast->sym->content->func, "exp") == 0) {
		return ast_mul(ast_dwrt(arg, var), ast_exp(ast_copy(arg)));
	} else if(strcmp(ast->sym->content->func, "log") == 0) {
		return ast_mul(ast_dwrt(arg, var),
		 ast_frac(ast_alloc(num_alloc(1)), ast_copy(arg)));
	} else if(strcmp(ast->sym->content->func, "sin") == 0) {
		return ast_mul(ast_dwrt(arg, var), ast_cos(ast_copy(arg)));
	} else if(strcmp(ast->sym->content->func, "sinh") == 0) {
		return ast_mul(ast_dwrt(arg, var), ast_cosh(ast_copy(arg)));
	} else if(strcmp(ast->sym->content->func, "tan") == 0) {
		return ast_sum(ast_alloc(num_alloc(1)), ast_expt(ast_tan(ast_copy(arg)), ast_alloc(num_alloc(2))));
	} else if(strcmp(ast->sym->content->func, "tanh") == 0) {
		return ast_sub(ast_expt(ast_tanh(ast_copy(arg)), ast_alloc(num_alloc(2))), ast_alloc(num_alloc(1)));
	}
	return NULL;
}

static Node*
ast_dwrt_op(Node *ast, char var)
{
	Node *diff, *expr;

	if(ast->sym->content->op == '+') {
		/* d/dx x + y = (d/dx x) + (d/dx y) */
		return ast_sum(ast_dwrt(ast->left, var), ast_dwrt(ast->right, var));;
	} else if(ast->sym->content->op == '-') {
		/* d/dx x - y = (d/dx x) - (d/dx y) */
		return ast_sub(ast_dwrt(ast->left, var), ast_dwrt(ast->right, var));
	} else if(ast->sym->content->op == '*') {
		/* d/dx x * y = (d/dx x) * y + (d/dx y) * x */
		return ast_sum(ast_mul(ast_copy(ast->right), ast_dwrt(ast->left, var)),
		 ast_mul(ast_copy(ast->left), ast_dwrt(ast->right, var)));
	} else if(ast->sym->content->op == '/') {
		/* d/dx x / y = [(d/dx x) * y - (d/dx y) * x] / y ^ 2 */
		return ast_frac(ast_sub(ast_mul(ast_copy(ast->right), ast_dwrt(ast->left, var)),
			  ast_mul(ast_copy(ast->left), ast_dwrt(ast->right, var))),
		  ast_expt(ast_copy(ast->right), ast_alloc(num_alloc(2))));
	} else if(ast->sym->content->op == '^') {
		if(is_num(ast->right->sym) && is_num(ast->left->sym)) {
			/* d/dx n^m = 0 */
			return ast_alloc(num_alloc(0));
		} else if(is_num(ast->right->sym)) {
			/* d/dx x ^ n = n * x ^ (n - 1) */
			return ast_mul(ast_copy(ast->right),
		  ast_expt(ast_copy(ast->left), ast_alloc(num_alloc(ast->right->sym->content->num - 1))));
		} else {
			/* d/dx x ^ f(x) = d/dx exp(f(x) * log(x)) */
			/* Workaround not to lose memory */
			expr = ast_exp(ast_mul(ast_copy(ast->right), ast_log(ast_copy(ast->left))));
			diff = ast_dwrt(expr, var);
			ast_free(expr);
			return diff;
		}
	}
	return NULL;
}

static Node*
ast_exp(Node *x)
{
	Node *exp;
	char *f;

	if(x == NULL)
		return NULL;

	f = ecalloc(4, sizeof(char));
	f = strcpy(f, "exp");
	exp = ast_alloc(func_alloc(f));
	ast_insert(exp, x);
	return exp;
}

static Node*
ast_expt(Node *x, Node *y)
{
	Node *expt;
	if(x == NULL || y == NULL)
		return NULL;

	if(num_equal(x->sym, 1)
		|| num_equal(y->sym, 0)) {
		ast_free(x);
		ast_free(y);
		return ast_alloc(num_alloc(1));
	} else if(num_equal(y->sym, 1)) {
		ast_free(y);
		return x;
	} else if(is_num(x->sym) && is_num(y->sym)) {
		expt = ast_alloc(num_alloc(pow(x->sym->content->num, y->sym->content->num)));
		ast_free(x);
		ast_free(y);
		return expt;
	} else {
		expt = ast_alloc(operator_alloc('^'));
		ast_insert(expt, y);
		ast_insert(expt, x);
		return expt;
	}
}


static Node*
ast_frac(Node *x, Node *y)
{
	Node *ast_frac;

	if(is_num(x->sym) && num_equal(x->sym, 0)) {
		ast_free(y);
		return x;
	} else if(is_num(y->sym) && num_equal(y->sym, 1)) {
		ast_free(y);
		return x;
	} else if(is_num(y->sym) && num_equal(y->sym, 0)) {
		ast_free(y);
		ast_free(x);
		return NULL;
	} else if(is_num(y->sym) && is_num(x->sym)) {
		/*
		 * Error handling when dividing by zero? How do I propagate the error?
		 */
		x->sym->content->num /= y->sym->content->num;
		ast_free(y);
		return x;
	} else {
		ast_frac = ast_alloc(operator_alloc('/'));
		ast_insert(ast_frac, y);
		ast_insert(ast_frac, x);
		return ast_frac;
	}
}

static Node*
ast_log(Node *x)
{
	Node *log;
	char *f;

	if(x == NULL)
		return NULL;

	f = ecalloc(4, sizeof(char));
	f = strcpy(f, "log");
	log = ast_alloc(func_alloc(f));
	ast_insert(log, x);
	return log;
}

static Node*
ast_mul(Node *x, Node *y)
{
	Node *ast_mul;

	if(x == NULL || y == NULL)
		return NULL;

	if(is_num(x->sym) && num_equal(x->sym, 1)) {
		ast_free(x);
		return y;
	} else if(is_num(y->sym) && num_equal(y->sym, 1)) {
		ast_free(y);
		return x;
	} else if(is_num(x->sym) && num_equal(x->sym, 0)) {
		ast_free(y);
		return x;
	} else if(is_num(y->sym) && num_equal(y->sym, 0)) {
		ast_free(x);
		return y;
	} else if(is_num(y->sym) && is_num(x->sym)) {
		x->sym->content->num *= y->sym->content->num;
		ast_free(y);
		return x;
	} else {
		ast_mul = ast_alloc(operator_alloc('*'));
		ast_insert(ast_mul, y);
		ast_insert(ast_mul, x);
		return ast_mul;
	}
}

static Node*
ast_sin(Node *x)
{
	Node *sin;
	char *f;

	if(x == NULL)
		return x;

	f = ecalloc(4, sizeof(char));
	f = strcpy(f, "sin");
	sin = ast_alloc(func_alloc(f));
	ast_insert(sin, x);
	return sin;
}

static Node*
ast_sinh(Node *x)
{
	Node *sinh;
	char *f;

	if(x == NULL)
		return x;

	f = ecalloc(5, sizeof(char));
	f = strcpy(f, "sinh");
	sinh = ast_alloc(func_alloc(f));
	ast_insert(sinh, x);
	return sinh;
}

static Node*
ast_sum(Node *x, Node *y)
{
	Node *ast_sum;

	if(is_num(x->sym) && num_equal(x->sym, 0)) {
		ast_free(x);
		return y;
	} else if(is_num(y->sym) && num_equal(y->sym, 0)) {
		ast_free(y);
		return x;
	} else if(is_num(y->sym) && is_num(x->sym)) {
		 x->sym->content->num += y->sym->content->num;
		 ast_free(y);
		 return x;
	} else {
		ast_sum = ast_alloc(operator_alloc('+'));
		ast_insert(ast_sum, y);
		ast_insert(ast_sum, x);
		return ast_sum;
	}
}

static Node*
ast_sub(Node *x, Node *y)
{
	Node *ast_sub;

	if(is_num(x->sym) && num_equal(x->sym, 0)) {
		ast_free(x);
		return ast_mul(ast_alloc(num_alloc(-1)), y);
	} else if(is_num(y->sym) && num_equal(y->sym, 0)) {
		ast_free(y);
		return x;
	} else if(is_num(y->sym) && is_num(x->sym)) {
		x->sym->content->num -= y->sym->content->num;
		ast_free(y);
		return x;
	} else {
		ast_sub = ast_alloc(operator_alloc('-'));
		ast_insert(ast_sub, y);
		ast_insert(ast_sub, x);
		return ast_sub;
	}
}

static Node*
ast_tan(Node *x)
{
	Node *tan;
	char *f;

	if(x == NULL)
	return x;

	f = ecalloc(4, sizeof(char));
	f = strcpy(f, "tan");
	tan = ast_alloc(func_alloc(f));
	ast_insert(tan, x);
	return tan;
}

static Node*
ast_tanh(Node *x)
{
	Node *tanh;
	char *f;

	if(x == NULL)
	return x;

	f = ecalloc(5, sizeof(char));
	f = strcpy(f, "tanh");
	tanh = ast_alloc(func_alloc(f));
	ast_insert(tanh, x);
	return tanh;
}

static int
is_same_var(Symbol *sym, char var)
{
	if(sym->type != S_VAR)
		return 0;
	return sym->content->var == var;
}
