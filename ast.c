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
#include <stdint.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static void 	ast_print_rec(Node*, Symbol*);
static void 	ast_to_latex_rec(Node*, Symbol*);
static char*	bit_to_func(uint8_t);
static char 	bit_to_op(uint8_t);
static uint8_t 	func_to_bit(char*);
static uint8_t 	op_to_bit(char);
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

/*
 * Returns a pointer to a heap-allocated deep copy of src
 */
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

/*
 * Insert new as child of ast. Try to insert it as right child, then fallback
 * to left child. If both right and left already point to something, left will
 * be overwritten.
 */
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
		printf("%c", node->sym->content.var);
		return;
	case S_NUM:
		printf("%.2f", node->sym->content.num);
		return;
	case S_FUNC:
		printf("%s(", bit_to_func(node->sym->content.func));
		ast_print_rec(node->right, node->sym);
		printf(")");
		return;
	case S_OP:
		if(previous != NULL && precedence(previous) > precedence(node->sym))
				printf("(");

		ast_print_rec(node->left, node->sym);

		printf(" %c ", bit_to_op(node->sym->content.func));

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
		printf("%c", ast->sym->content.var);
		return;
	case S_NUM:
		printf("%.2f", ast->sym->content.num);
		return;
	case S_FUNC:
		printf("\\%s\\left(", bit_to_func(ast->sym->content.func));
		ast_to_latex(ast->right);
		printf("\\right)");
		return;
	case S_OP:
		if(precedence(previous) > precedence(ast->sym))
				printf("\\left(");

		switch(bit_to_op(ast->sym->content.func)) {
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
			printf("%c", bit_to_op(ast->sym->content.func));
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

static char*
bit_to_func(uint8_t bit)
{
	size_t i;
	for(i = 0; i < KNOWN_FUNCS; i++)
		if(bit == known_funcs[i].bit)
			return known_funcs[i].func;
	return "";
}

static char
bit_to_op(uint8_t bit)
{
	size_t i;
	for(i = 0; i < KNOWN_OPERATORS; i++)
		if(bit == known_operators[i].bit)
			return known_operators[i].op;
	return '\0';
}

Symbol*
func_alloc(char *func)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content.func = func_to_bit(func);
	sym->type = S_FUNC;
	return sym;
}

static uint8_t
func_to_bit(char *func)
{
	size_t i;
	for(i = 0; i < KNOWN_FUNCS; i++)
		if(strcmp(func, known_funcs[i].func) == 0)
			return known_funcs[i].bit;
	return 0xFF;
}

int
is_same_var(Symbol *sym, char var)
{
	if(sym->type != S_VAR)
		return 0;
	return sym->content.var == var;
}

Symbol*
lparen_alloc(void)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content.func = 0xFF;
	sym->type = S_LPAREN;
	return sym;
}

Symbol*
num_alloc(double num)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content.num = num;
	sym->type = S_NUM;
	return sym;
}

Symbol*
operator_alloc(char op)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content.func = op_to_bit(op);
	sym->type = S_OP;
	return sym;
}

static uint8_t
op_to_bit(char op)
{
	size_t i;
	for(i = 0; i < KNOWN_OPERATORS; i++)
		if(op == known_operators[i].op)
			return known_operators[i].bit;
	return 0xFF;
}

Symbol*
rparen_alloc(void)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content.func = 0xFF;
	sym->type = S_RPAREN;
	return sym;
}

Symbol*
var_alloc(char var)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content.var = var;
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
	return is_num(sym) ? fabs(sym->content.num - num) < 1e-7 : 0;
}

void
symbol_free(Symbol *sym)
{
	if(sym == NULL)
		return;
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
		/* TODO: Fix this
		 * symbol_copy tries to free symbol->content.func, so heap
		 * allocate this
		 */
		dest = func_alloc(bit_to_func(src->content.func));
		break;
	case S_OP:
		dest = operator_alloc(src->content.func);
		break;
	case S_VAR:
		dest = var_alloc(src->content.var);
		break;
	case S_NUM:
		dest = num_alloc(src->content.num);
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
		printf("Function: %s\n", bit_to_func(sym->content.func));
		break;
	case S_VAR:
		printf("Variable: %c\n", sym->content.var);
		break;
	case S_NUM:
		printf("Number: %f\n", sym->content.num);
		break;
	case S_OP:
		printf("Operator: %c\n", bit_to_op(sym->content.func));
		break;
	default:
		fprintf(stderr, "Unknown symbol type %d\n", sym->type);
	}
}
