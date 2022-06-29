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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static Node*	ast_cos(Node*);
static Node*	ast_cosh(Node*);
static Node*	ast_derive(Node*, char);
static Node*	ast_frac(Node*, Node*);
static Node*	ast_mul(Node*, Node*);
static Node*	ast_sin(Node*);
static Node*	ast_sinh(Node*);
static Node*	ast_sum(Node*, Node*);
static Node*	ast_sub(Node*, Node*);
static int	is_same_var(Symbol*, char);
static void	usage(char*);

static Node*
ast_cos(Node *x)
{
	Node *cos;
	if(x == NULL)
		return NULL;

	cos = ast_alloc(func_alloc("cos"));
	ast_insert(cos, x);
	return cos;
}

static Node*
ast_cosh(Node *x)
{
	Node *cosh;

	if(x == NULL)
		return NULL;

	cosh = ast_alloc(func_alloc("cosh"));
	ast_insert(cosh, x);
	return cosh;
}

/*
 * TODO: symplify numerical expressions
 */
static Node*
ast_derive(Node *ast, char var)
{
	Node *arg, *diff;

	diff = NULL;
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
		if(ast->sym->content->op == '+') {
			/* d/dx x + y = (d/dx x) + (d/dx y) */
			diff = ast_sum(ast_derive(ast->left, var), ast_derive(ast->right, var));
			return diff;
		} else if(ast->sym->content->op == '-') {
			/* d/dx x - y = (d/dx x) - (d/dx y) */
			diff = ast_sub(ast_derive(ast->left, var), ast_derive(ast->right, var));
			return diff;
		} else if(ast->sym->content->op == '*') {
			/* d/dx x * y = (d/dx x) * y + (d/dx y) * x */
			diff = ast_sum(ast_mul(ast_copy(ast->right), ast_derive(ast->left, var)),
				   ast_mul(ast_copy(ast->left), ast_derive(ast->right, var)));
			return diff;
		} else if(ast->sym->content->op == '/') {
			/* d/dx x / y = [(d/dx x) * y - (d/dx y) * x] / y ^ 2 */
			diff = ast_frac(ast_sub(ast_mul(ast_copy(ast->right), ast_derive(ast->left, var)),
					ast_mul(ast_copy(ast->left), ast_derive(ast->right, var))),
				    ast_mul(ast_copy(ast->right), ast_copy(ast->right)));

			return diff;
		}
		break;
	case S_FUNC:
		arg = ast->right;
		if(strcmp(ast->sym->content->func, "cos") == 0) {
			return ast_mul(ast_derive(arg, var),
				       ast_mul(ast_alloc(num_alloc(-1)), ast_sin(ast_copy(arg))));
		} else if(strcmp(ast->sym->content->func, "cosh") == 0) {
			return ast_mul(ast_derive(arg, var), ast_sinh(ast_copy(arg)));
		} else if(strcmp(ast->sym->content->func, "sin") == 0) {
			return ast_mul(ast_derive(arg, var), ast_cos(ast_copy(arg)));
		} else if(strcmp(ast->sym->content->func, "sinh") == 0) {
			return ast_mul(ast_derive(arg, var), ast_cosh(ast_copy(arg)));
		} else if(strcmp(ast->sym->content->func, "tan") == 0) {
			return ast_frac(ast_derive(arg, var),
					ast_mul(ast_cos(ast_copy(arg)),
						ast_cos(ast_copy(arg))));
		} else if(strcmp(ast->sym->content->func, "tanh") == 0) {
			return ast_frac(ast_derive(arg, var),
					ast_mul(ast_cosh(ast_copy(arg)), ast_cosh(ast_copy(arg))));
		}
	default:
		break;
	}
	return diff;
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
ast_mul(Node *x, Node *y)
{
	Node *ast_mul;

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
		ast_insert(ast_mul, x);
		ast_insert(ast_mul, y);
		return ast_mul;
	}
}

static Node*
ast_sin(Node *x)
{
	Node *sin;

	if(x == NULL)
		return x;
	sin = ast_alloc(func_alloc("sin"));
	ast_insert(sin, x);
	return sin;
}

static Node*
ast_sinh(Node *x)
{
	Node *sinh;

	if(x == NULL)
		return x;
	sinh = ast_alloc(func_alloc("sinh"));
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
		ast_insert(ast_sum, x);
		ast_insert(ast_sum, y);
		return ast_sum;
	}
}

static Node*
ast_sub(Node *x, Node *y)
{
	Node *ast_sub;

	if(is_num(x->sym) && num_equal(x->sym, 0)) {
		ast_free(x);
		return y;
	} else if(is_num(y->sym) && num_equal(y->sym, 0)) {
		ast_free(y);
		return x;
	} else if(is_num(y->sym) && is_num(x->sym)) {
		x->sym->content->num -= y->sym->content->num;
		ast_free(y);
		return x;
	} else {
		ast_sub = ast_alloc(operator_alloc('-'));
		ast_insert(ast_sub, x);
		ast_insert(ast_sub, y);
		return ast_sub;
	}
}

static int
is_same_var(Symbol *sym, char var)
{
	if(sym->type != S_VAR)
		return 0;
	return sym->content->var == var;
}

static void
usage(char *arg0)
{
	fprintf(stderr, "usage: %s variable\n", arg0);
}

int
main(int argc, char *argv[])
{
	Parser *p;
	Node *diff;

	if(argc != 2) {
		usage(argv[0]);
		exit(1);
	}

	p = p_init("./test.txt");
	diff = NULL;

	if(parse(p) < 0) {
		fprintf(stderr, p->err);
		p_free(p);
		exit(1);
	}

	diff = ast_derive(p->ast, argv[1][0]);
	ast_print(diff);
	ast_free(diff);
	p_free(p);

	return 0;
}
