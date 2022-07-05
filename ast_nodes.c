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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

Node*
ast_cos(Node *x)
{
	Node *cos;
	if(x == NULL)
		return NULL;

	cos = ast_alloc(func_alloc("cos"));
	ast_insert(cos, x);
	return cos;
}

Node*
ast_cosh(Node *x)
{
	Node *cosh;

	if(x == NULL)
		return NULL;

	cosh = ast_alloc(func_alloc("cosh"));
	ast_insert(cosh, x);
	return cosh;
}

Node*
ast_exp(Node *x)
{
	Node *exp;

	if(x == NULL)
		return NULL;

	exp = ast_alloc(func_alloc("exp"));
	ast_insert(exp, x);
	return exp;
}

Node*
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
		expt = ast_alloc(num_alloc(pow(x->sym->content.num, y->sym->content.num)));
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

Node*
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
		x->sym->content.num /= y->sym->content.num;
		ast_free(y);
		return x;
	} else {
		ast_frac = ast_alloc(operator_alloc('/'));
		ast_insert(ast_frac, y);
		ast_insert(ast_frac, x);
		return ast_frac;
	}
}

Node*
ast_log(Node *x)
{
	Node *log;

	if(x == NULL)
		return NULL;

	log = ast_alloc(func_alloc("log"));
	ast_insert(log, x);
	return log;
}

Node*
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
		x->sym->content.num *= y->sym->content.num;
		ast_free(y);
		return x;
	} else {
		ast_mul = ast_alloc(operator_alloc('*'));
		ast_insert(ast_mul, y);
		ast_insert(ast_mul, x);
		return ast_mul;
	}
}

Node*
ast_sin(Node *x)
{
	Node *sin;

	if(x == NULL)
		return x;

	sin = ast_alloc(func_alloc("sin"));
	ast_insert(sin, x);
	return sin;
}

Node*
ast_sinh(Node *x)
{
	Node *sinh;

	if(x == NULL)
		return x;

	sinh = ast_alloc(func_alloc("sinh"));
	ast_insert(sinh, x);
	return sinh;
}

Node*
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
		x->sym->content.num -= y->sym->content.num;
		ast_free(y);
		return x;
	} else {
		ast_sub = ast_alloc(operator_alloc('-'));
		ast_insert(ast_sub, y);
		ast_insert(ast_sub, x);
		return ast_sub;
	}
}

Node*
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
		 x->sym->content.num += y->sym->content.num;
		 ast_free(y);
		 return x;
	} else {
		ast_sum = ast_alloc(operator_alloc('+'));
		ast_insert(ast_sum, y);
		ast_insert(ast_sum, x);
		return ast_sum;
	}
}

Node*
ast_tan(Node *x)
{
	Node *tan;

	if(x == NULL)
	return x;

	tan = ast_alloc(func_alloc("tan"));
	ast_insert(tan, x);
	return tan;
}

Node*
ast_tanh(Node *x)
{
	Node *tanh;

	if(x == NULL)
	return x;

	tanh = ast_alloc(func_alloc("tanh"));
	ast_insert(tanh, x);
	return tanh;
}
