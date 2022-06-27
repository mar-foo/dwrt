#include <stdio.h>
#include <stdlib.h>

#include "dat.h"
#include "fns.h"

Node*
alloc_node(Symbol *sym)
{
	Node *node;

	node = ecalloc(1, sizeof(Node)); /* node->left = node->right = NULL */
	node->sym = sym;
	return node;
}

Symbol*
alloc_func(Function *func)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));

	sym->content->func = func;
	sym->type = S_FUNC;
	return sym;
}

Symbol*
alloc_number(double num)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content->num = num;
	sym->type = S_NUM;
	return sym;
}

Symbol*
alloc_var(char var)
{
	Symbol *sym;

	sym = emalloc(sizeof(Symbol));
	sym->content->var = var;
	sym->type = S_VAR;
	return sym;
}

void
print_symbol(Symbol *sym)
{
	if(sym == NULL) {
		printf("_");
		return;
	}

	switch(sym->type) {
	case S_FUNC:
		printf("Function: %s", sym->content->func->name);
		break;
	case S_VAR:
		printf("Variable: %c", sym->content->var);
		break;
	case S_NUM:
		printf("Number: %f", sym->content->num);
		break;
	default:
		fprintf(stderr, "Unknown symbol type %d", sym->type);
	}
}

void
print_tree(Node *root)
{
	if(root == NULL) {
		printf("%s", "<empty>");
		return;
	}
	print_symbol(root->sym);
	printf("left");
	print_tree(root->left);
	printf("right");
	print_tree(root->right);
}
