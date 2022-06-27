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
static void	print_tree_rec(Node*, int);

Node*
alloc_node(Symbol *sym)
{
	Node *node;

	node = ecalloc(1, sizeof(Node)); /* node->left = node->right = NULL */
	node->sym = sym;
	return node;
}

Symbol*
alloc_func(char *func)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->func = func;
	sym->type = S_FUNC;
	return sym;
}

Symbol*
alloc_lparen()
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->func = "(";
	sym->type = S_LPAREN;
	return sym;
}

Symbol*
alloc_num(double num)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->num = num;
	sym->type = S_NUM;
	return sym;
}

Symbol*
alloc_operator(char op)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->op = op;
	sym->type = S_OP;
	return sym;
}

Symbol*
alloc_rparen()
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content = emalloc(sizeof *sym->content);
	sym->content->func = ")";
	sym->type = S_RPAREN;
	return sym;
}

Symbol*
alloc_var(char var)
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

Node*
ast_insert(Node *ast, Node *new)
{
	new->parent = ast;
	if(ast->left != NULL)
		return ast->right = new;
	return ast->left = new;
}

Node*
ast_insert_above(Node *ast, Node *new)
{
	new->parent = ast->parent;
	ast->parent = new;
	new->left = ast;
	if(new->parent->left == ast)
		new->parent->left = new;
	else
		new->parent->right = new;
	return new;
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
print_symbol(Symbol *sym)
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
print_tree_rec(Node *root, int level)
{
	print_tabs(level);
	if(root == NULL) {
		printf("%s", "<empty>\n");
		return;
	}
	print_symbol(root->sym);

	print_tabs(level);
	printf("left\n");
	print_tree_rec(root->left, level + 1);

	print_tabs(level);
	printf("right\n");
	print_tree_rec(root->right, level + 1);
}

void
print_tree(Node *root)
{
	print_tree_rec(root, 0);
}

void
symbol_cleanup(Symbol *sym)
{
	if(sym->type == S_FUNC)
		free(sym->content->func);
	free(sym->content);
	free(sym);
}
