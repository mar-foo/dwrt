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

static void	print_tabs(int);
static void	ast_print_rec(Node*, int);

Node*
ast_alloc(Symbol *sym)
{
	Node *node;

	node = ecalloc(1, sizeof(Node)); /* node->left = node->right = NULL */
	node->sym = sym;
	return node;
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
lparen_alloc()
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
rparen_alloc()
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

void
ast_cleanup(Node *ast)
{
	if(ast == NULL)
		return;
	symbol_cleanup(ast->sym);
	ast_cleanup(ast->right);
	ast_cleanup(ast->left);
	free(ast);
}

void
ast_insert(Node *ast, Node *new)
{
	new->parent = ast;
	if(ast->right != NULL) {
		ast->left = new;
		return;
	}
	ast->right = new;
}

int
is_lparen(Symbol *sym)
{
	return sym == NULL ? 0 : sym->type == S_LPAREN;
}

int
is_function(Symbol *sym)
{
	return sym == NULL ? 0 : sym->type == S_FUNC;
}

int
is_operator(Symbol *sym)
{
	if(sym->type == S_FUNC)
		return strcmp(sym->content->func, "+") &&
			strcmp(sym->content->func, "-") &&
			strcmp(sym->content->func, "*") &&
			strcmp(sym->content->func, "/");
	return 0;
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
	default:
		fprintf(stderr, "Unknown symbol type %d\n", sym->type);
	}
}

static void
print_tabs(int tabs)
{
	int i;
	for(i = 0; i < tabs; i++) printf("\t");
}

static void
ast_print_rec(Node *root, int level)
{
	print_tabs(level);
	if(root == NULL) {
		printf("%s", "<empty>\n");
		return;
	}
	symbol_print(root->sym);

	print_tabs(level);
	printf("left\n");
	ast_print_rec(root->left, level + 1);

	print_tabs(level);
	printf("right\n");
	ast_print_rec(root->right, level + 1);
}

void
ast_print(Node *root)
{
	ast_print_rec(root, 0);
}

void
symbol_cleanup(Symbol *sym)
{
	if(sym == NULL)
		return;
	if(sym->type == S_FUNC)
		free(sym->content->func);
	free(sym->content);
	free(sym);
}
