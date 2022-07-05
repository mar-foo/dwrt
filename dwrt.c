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

#define LEN(x) (sizeof((x)) / sizeof((x)[0]))

static Node*	ast_dwrt_cos(Node*, char);
static Node*	ast_dwrt_cosh(Node*, char);
static Node*	ast_dwrt_exp(Node*, char);
static Node*	ast_dwrt_expt(Node*, char);
static Node*	ast_dwrt_frac(Node*, char);
static Node*	ast_dwrt_func(Node*, char);
static Node*	ast_dwrt_log(Node*, char);
static Node*	ast_dwrt_mul(Node*, char);
static Node*	ast_dwrt_op(Node*, char);
static Node*	ast_dwrt_sin(Node*, char);
static Node*	ast_dwrt_sinh(Node*, char);
static Node*	ast_dwrt_sub(Node*, char);
static Node*	ast_dwrt_sum(Node*, char);
static Node*	ast_dwrt_tan(Node*, char);
static Node*	ast_dwrt_tanh(Node*, char);

/* TODO: Make this a hash map */
static struct derivative {
	char *func;
	Derivative derivative;
} func_derivatives[] = {
	{"cos", ast_dwrt_cos},
	{"cosh", ast_dwrt_cosh},
	{"exp", ast_dwrt_exp},
	{"log", ast_dwrt_log},
	{"sin", ast_dwrt_sin},
	{"sinh", ast_dwrt_sinh},
	{"tan", ast_dwrt_tan},
	{"tanh", ast_dwrt_tanh}
};

static struct op_derivative {
	char op;
	Derivative derivative;
} op_derivatives[] = {
	{'-', ast_dwrt_sub},
	{'+', ast_dwrt_sum},
	{'^', ast_dwrt_expt},
	{'/', ast_dwrt_frac},
	{'*', ast_dwrt_mul},
};

static Node*
ast_dwrt_cos(Node *arg, char var)
{
	return ast_mul(ast_dwrt(arg, var),
	 ast_mul(ast_alloc(num_alloc(-1)), ast_sin(ast_copy(arg))));
}

static Node*
ast_dwrt_cosh(Node *arg, char var)
{
	return ast_mul(ast_dwrt(arg, var), ast_sinh(ast_copy(arg)));
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
ast_dwrt_exp(Node *arg, char var)
{
	return ast_mul(ast_dwrt(arg, var), ast_exp(ast_copy(arg)));
}

static Node*
ast_dwrt_expt(Node *ast, char var)
{
	Node *diff, *expr;
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

static Node*
ast_dwrt_frac(Node *ast, char var)
{
	/* d/dx x / y = [(d/dx x) * y - (d/dx y) * x] / y ^ 2 */
	return ast_frac(ast_sub(ast_mul(ast_copy(ast->right), ast_dwrt(ast->left, var)),
		  ast_mul(ast_copy(ast->left), ast_dwrt(ast->right, var))),
	  ast_expt(ast_copy(ast->right), ast_alloc(num_alloc(2))));

}

/* TODO: Make some kind of table to store function -> derivative association
 * Differentiate function nodes with respect to var
 */
static Node*
ast_dwrt_func(Node *ast, char var)
{
	size_t i;
	Node *arg;

	arg = ast->right;

	for(i = 0; i < LEN(func_derivatives); i++)
		if(strcmp(func_derivatives[i].func, ast->sym->content->func) == 0)
			return func_derivatives[i].derivative(arg, var);

	return NULL;
}

static Node*
ast_dwrt_log(Node *arg, char var)
{
	return ast_mul(ast_dwrt(arg, var),
	 ast_frac(ast_alloc(num_alloc(1)), ast_copy(arg)));
}

static Node*
ast_dwrt_mul(Node *ast, char var)
{
	/* d/dx x * y = (d/dx x) * y + (d/dx y) * x */
	return ast_sum(ast_mul(ast_copy(ast->right), ast_dwrt(ast->left, var)),
	 ast_mul(ast_copy(ast->left), ast_dwrt(ast->right, var)));
}

/*
 * Differentiate operator nodes (+, -, *, /, ^) with respect to var
 */
static Node*
ast_dwrt_op(Node *ast, char var)
{
	size_t i;
	Node *diff, *expr;

	for(i = 0; i < LEN(op_derivatives); i++)
		if(op_derivatives[i].op == ast->sym->content->op)
			return op_derivatives[i].derivative(ast, var);

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
ast_dwrt_sin(Node *arg, char var)
{
	return ast_mul(ast_dwrt(arg, var), ast_cos(ast_copy(arg)));
}

static Node*
ast_dwrt_sinh(Node *arg, char var)
{
	return ast_mul(ast_dwrt(arg, var), ast_cosh(ast_copy(arg)));
}

static Node*
ast_dwrt_sub(Node *ast, char var)
{
	/* d/dx x - y = (d/dx x) - (d/dx y) */
	return ast_sub(ast_dwrt(ast->left, var), ast_dwrt(ast->right, var));
}

static Node*
ast_dwrt_sum(Node *ast, char var)
{
	/* d/dx x + y = (d/dx x) + (d/dx y) */
	return ast_sum(ast_dwrt(ast->left, var), ast_dwrt(ast->right, var));
}

static Node*
ast_dwrt_tan(Node *arg, char var)
{
	return ast_mul(ast_dwrt(arg, var),
		ast_sum(ast_alloc(num_alloc(1)), ast_expt(ast_tan(ast_copy(arg)),
	ast_alloc(num_alloc(2)))));
}

static Node*
ast_dwrt_tanh(Node *arg, char var)
{
	return ast_mul(ast_dwrt(arg, var),
		ast_sub(ast_expt(ast_tanh(ast_copy(arg)), ast_alloc(num_alloc(2))),
	  ast_alloc(num_alloc(1))));
}
