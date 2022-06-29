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

static Node*	ast_derive(Node*, char);
static int	is_same_var(Symbol*, char);
static Node*	frac(Node*, Node*);
static Node*	mul(Node*, Node*);
static Node*	sum(Node*, Node*);
static Node*	sub(Node*, Node*);
static void	usage(char*);

/*
 * TODO: symplify numerical expressions
 */
static Node*
ast_derive(Node *ast, char var)
{
	Node *diff;

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
			diff = sum(ast_derive(ast->left, var), ast_derive(ast->right, var));
			return diff;
		} else if(ast->sym->content->op == '-') {
			/* d/dx x - y = (d/dx x) - (d/dx y) */
			diff = sub(ast_derive(ast->left, var), ast_derive(ast->right, var));
			return diff;
		} else if(ast->sym->content->op == '*') {
			/* d/dx x * y = (d/dx x) * y + (d/dx y) * x */
			diff = sum(mul(ast_copy(ast->right), ast_derive(ast->left, var)),
				   mul(ast_copy(ast->left), ast_derive(ast->right, var)));
			return diff;
		} else if(ast->sym->content->op == '/') {
			/* d/dx x / y = [(d/dx x) * y - (d/dx y) * x] / y ^ 2 */
			diff = frac(sub(mul(ast_copy(ast->right), ast_derive(ast->left, var)),
					mul(ast_copy(ast->left), ast_derive(ast->right, var))),
				    mul(ast_copy(ast->right), ast_copy(ast->right)));

			return diff;
		}
		break;
	default:
		break;
	}
	return diff;
}

static Node*
frac(Node *x, Node *y)
{
	Node *frac;

	if(is_num(x->sym) && num_equal(x->sym, 0)) {
		ast_cleanup(y);
		return x;
	} else if(is_num(y->sym) && num_equal(y->sym, 1)) {
		ast_cleanup(y);
		return x;
	} else if(is_num(y->sym) && is_num(x->sym)) {
		/*
		 * Error handling when dividing by zero? How do I propagate the error?
		 */
		x->sym->content->num /= y->sym->content->num;
		ast_cleanup(y);
		return x;
	} else {
		frac = ast_alloc(operator_alloc('/'));
		ast_insert(frac, y);
		ast_insert(frac, x);
		return frac;
	}
}

static int
is_same_var(Symbol *sym, char var)
{
	if(sym->type != S_VAR)
		return 0;
	return sym->content->var == var;
}

static Node*
mul(Node *x, Node *y)
{
	Node *mul;

	if(is_num(x->sym) && num_equal(x->sym, 1)) {
		ast_cleanup(x);
		return y;
	} else if(is_num(y->sym) && num_equal(y->sym, 1)) {
		ast_cleanup(y);
		return x;
	} else if(is_num(x->sym) && num_equal(x->sym, 0)) {
		ast_cleanup(y);
		return x;
	} else if(is_num(y->sym) && num_equal(y->sym, 0)) {
		ast_cleanup(x);
		return y;
	} else if(is_num(y->sym) && is_num(x->sym)) {
		x->sym->content->num *= y->sym->content->num;
		ast_cleanup(y);
		return x;
	} else {
		mul = ast_alloc(operator_alloc('*'));
		ast_insert(mul, x);
		ast_insert(mul, y);
		return mul;
	}
}

static Node*
sum(Node *x, Node *y)
{
	Node *sum;

	if(is_num(x->sym) && num_equal(x->sym, 0)) {
		ast_cleanup(x);
		return y;
	} else if(is_num(y->sym) && num_equal(y->sym, 0)) {
		ast_cleanup(y);
		return x;
	} else if(is_num(y->sym) && is_num(x->sym)) {
		 x->sym->content->num += y->sym->content->num;
		 ast_cleanup(y);
		 return x;
	} else {
		sum = ast_alloc(operator_alloc('+'));
		ast_insert(sum, x);
		ast_insert(sum, y);
		return sum;
	}
}

static Node*
sub(Node *x, Node *y)
{
	Node *sub;

	if(is_num(x->sym) && num_equal(x->sym, 0)) {
		ast_cleanup(x);
		return y;
	} else if(is_num(y->sym) && num_equal(y->sym, 0)) {
		ast_cleanup(y);
		return x;
	} else if(is_num(y->sym) && is_num(x->sym)) {
		x->sym->content->num -= y->sym->content->num;
		ast_cleanup(y);
		return x;
	} else {
		sub = ast_alloc(operator_alloc('-'));
		ast_insert(sub, x);
		ast_insert(sub, y);
		return sub;
	}
}

static void
usage(char *arg0)
{
	fprintf(stderr, "%s: variable\n", arg0);
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
		ast_cleanup(diff);
		p_cleanup(p);
		exit(1);
	}

	diff = ast_derive(p->ast, argv[1][0]);
	ast_print(diff);
	ast_cleanup(diff);
	p_cleanup(p);

	return 0;
}
