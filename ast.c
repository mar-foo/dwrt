/*
 * Copyright ©️ 2022 Mario Forzanini <mf@marioforzanini.com>
 *
 * This file is part of dwrt.
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
 * along with dwrt. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static void 	ast_print_rec(Node*, Symbol*);
static void 	ast_to_latex_rec(Node*, Symbol*);
static Symbol*	symbol_copy(Symbol*);

#define MAX_FUNC_LENGTH 5

Node*
ast_alloc(Symbol *sym)
{
	Node *node;

	node = ecalloc(1, sizeof(Node)); /* node->left = node->right = NULL */
	node->sym = sym;
	return node;
}

void
ast_free(Node *ast)
{
	if(ast == NULL)
		return;
	symbol_free(ast->sym);
	ast_free(ast->right);
	ast_free(ast->left);
	free(ast);
}

Node*
ast_copy(Node *src)
{
	Node *dest;

	if(src == NULL)
		return NULL;
	dest = ast_alloc(symbol_copy(src->sym));

	dest->left = ast_copy(src->left);
	dest->right = ast_copy(src->right);
	return dest;
}

void
ast_insert(Node *ast, Node *new)
{
	if(new == NULL || ast == NULL)
		return;
	new->parent = ast;
	if(ast->right != NULL) {
		ast->left = new;
		return;
	}
	ast->right = new;
}

void
ast_print(Node *node)
{
	ast_print_rec(node, NULL);
}

static void
ast_print_rec(Node *node, Symbol *previous)
{
	if(node == NULL) return;

	switch(node->sym->type) {
	case S_VAR:
		printf("%c", node->sym->content->var);
		return;
	case S_NUM:
		printf("%.2f", node->sym->content->num);
		return;
	case S_FUNC:
		printf("%s(", node->sym->content->func);
		ast_print_rec(node->right, node->sym);
		printf(")");
		return;
	case S_OP:
		if(previous != NULL && precedence(previous) > precedence(node->sym))
				printf("(");

		ast_print_rec(node->left, node->sym);

		printf(" %c ", node->sym->content->op);

		ast_print_rec(node->right, node->sym);

		if(previous != NULL && precedence(previous) > precedence(node->sym))
				printf(")");
		return;
	default:
		return;
	}
}

void
ast_to_latex(Node *ast)
{
	ast_to_latex_rec(ast, NULL);
}

static void
ast_to_latex_rec(Node *ast, Symbol *previous)
{
	if(ast == NULL) return;

	switch(ast->sym->type) {
	case S_VAR:
		printf("%c", ast->sym->content->var);
		return;
	case S_NUM:
		printf("%.2f", ast->sym->content->num);
		return;
	case S_FUNC:
		printf("\\%s\\left(", ast->sym->content->func);
		ast_to_latex(ast->right);
		printf("\\right)");
		return;
	case S_OP:
		if(precedence(previous) > precedence(ast->sym))
				printf("\\left(");

		switch(ast->sym->content->op) {
		case '/':
			printf("\\frac{");
			ast_to_latex(ast->left);
			printf("}{");
			ast_to_latex(ast->right);
			printf("}");
			break;
		case '^':
			ast_to_latex(ast->left);
			printf("^{");
			ast_to_latex(ast->right);
			printf("}");
			break;
		default:
			ast_to_latex(ast->left);
			printf("%c", ast->sym->content->op);
			ast_to_latex(ast->right);
			break;
		}

		if(precedence(previous) > precedence(ast->sym))
				printf("\\left(");
		break;
	default:
		return;
	}
}

Symbol*
func_alloc(char *func)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->func = func;
	sym->type = S_FUNC;
	return sym;
}

Symbol*
lparen_alloc(void)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->func = "(";
	sym->type = S_LPAREN;
	return sym;
}

Symbol*
num_alloc(double num)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->num = num;
	sym->type = S_NUM;
	return sym;
}

Symbol*
operator_alloc(char op)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->op = op;
	sym->type = S_OP;
	return sym;
}

Symbol*
rparen_alloc(void)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->func = ")";
	sym->type = S_RPAREN;
	return sym;
}

Symbol*
var_alloc(char var)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->var = var;
	sym->type = S_VAR;
	return sym;
}

int
is_function(Symbol *sym)
{
	return sym == NULL ? 0 : sym->type == S_FUNC;
}

int
is_lparen(Symbol *sym)
{
	return sym == NULL ? 0 : sym->type == S_LPAREN;
}

int
is_operator(Symbol *sym)
{
	return sym == NULL ? 0 : sym->type == S_OP;
}

int
is_num(Symbol *sym)
{
	return sym == NULL ? 0 : sym->type == S_NUM;
}

int
num_equal(Symbol *sym, double num)
{
	return is_num(sym) ? fabs(sym->content->num - num) < 1e-7 : 0;
}

void
symbol_free(Symbol *sym)
{
	if(sym == NULL)
		return;
	if(sym->type == S_FUNC)
		free(sym->content->func);
	free(sym->content);
	free(sym);
}

static Symbol*
symbol_copy(Symbol *src)
{
	char *func;
	Symbol *dest;

	if(src == NULL)
		return NULL;

	dest = NULL;
	switch(src->type) {
	case S_FUNC:
		func = ecalloc(MAX_FUNC_LENGTH, sizeof(char));
		func = strcpy(func, src->content->func);
		dest = func_alloc(func);
		break;
	case S_OP:
		dest = operator_alloc(src->content->op);
		break;
	case S_VAR:
		dest = var_alloc(src->content->var);
		break;
	case S_NUM:
		dest = num_alloc(src->content->num);
		break;
	default:
		break;
	}
	return dest;
}

void
symbol_print(Symbol *sym)
{
	if(sym == NULL) {
		printf("NULL\n");
		return;
	}

	switch(sym->type) {
	case S_FUNC:
		printf("Function: %s\n", sym->content->func);
		break;
	case S_VAR:
		printf("Variable: %c\n", sym->content->var);
		break;
	case S_NUM:
		printf("Number: %f\n", sym->content->num);
		break;
	case S_OP:
		printf("Operator: %c\n", sym->content->op);
		break;
	default:
		fprintf(stderr, "Unknown symbol type %d\n", sym->type);
	}
}
