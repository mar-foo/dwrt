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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

#define KNOWN_FUNCS 8
#define LEXEME_MINSZ 10

enum stack_type {
	NODE,
	SYM
};

typedef struct Stack Stack;
struct Stack {
	void *data;
	enum stack_type type;
	Stack *next;
};

static char	l_getc(Lexer*);
static void*	peek(Stack*);
static void*	pop(Stack**);
static Stack*	push(Stack*, void*, enum stack_type);
static Stack*	stack_alloc(void*, enum stack_type);
static void	stack_free(Stack*);
static int	stack_len(Stack*);

static char *known_funcs[] = {"cos", "cosh", "exp", "log", "sin", "sinh", "tan", "tanh"};

static char
l_getc(Lexer *l)
{
	return *(l->pos++);
}

void
l_free(Lexer *lex)
{
	free(lex->data);
	free(lex->err);
	free(lex);
}

Lexer*
l_init(char *filename)
{
	FILE *f;
	Lexer *l;

	l = emalloc(sizeof(Lexer));

	if(filename == NULL) {
		l->filename = "stdin";

		f = stdin;
	} else {
		l->filename = filename;
		if((f = fopen(filename, "r")) == NULL) {
			free(l);
			return NULL;
		}
	}

	l->err = NULL;
	l->state = LS_WS;
	l->pos = l->data = readall(f);

	if(filename == NULL) fclose(f);

	return l;
}

Lexeme*
lex(Lexer *l)
{
	char c;
	size_t offset;
	Lexeme *result;

	result = emalloc(sizeof(Lexeme));
	result->len = LEXEME_MINSZ;
	result->lexeme = ecalloc(result->len, sizeof(char));
	result->type = LE_EOF;
	offset = 0;
	while((c = l_getc(l)) != EOF) {
		if(isspace(c)) {
			switch(l->state) {
			case LS_SYMBOL:
				l->state = LS_WS;
				return result;
			default:
				continue;
			}
		} else if(isalpha(c)) {
			switch(l->state) {
			case LS_WS:
				l->state = LS_SYMBOL;
				result->len = strappend(result->lexeme, c, offset++, result->len);
				result->type = LE_SYMBOL;
				break;
			case LS_SYMBOL:
				result->len = strappend(result->lexeme, c, offset++, result->len);
				result->type = LE_SYMBOL;
				break;
			default:
				l->pos--;
				l->state = LS_WS;
				return result;
			}
		} else if(isdigit(c)) {
			switch(l->state) {
			case LS_WS:
				l->state = LS_NUMBER;
				result->len = strappend(result->lexeme, c, offset++, result->len);
				result->type = LE_NUMBER;
				break;
			case LS_NUMBER:
				result->len = strappend(result->lexeme, c, offset++, result->len);
				result->type = LE_NUMBER;
				break;
			default:
				l->pos--;
				l->state = LS_WS;
				return result;
			}
		} else {
			switch(c) {
			case '+':
			case '-':
			case '*':
			case '^':
			case '/':
				switch(l->state) {
				case LS_WS:
					result->type = LE_OPERATOR;
					result->len = strappend(result->lexeme, c, offset++, result->len);
					return result;
				default:
					l->pos--;
					l->state = LS_WS;
					return result;
				}
				break;
			case '.':
				switch(l->state) {
				case LS_NUMBER:
					result->len = strappend(result->lexeme, c, offset++, result->len);
					break;
				default:
					l->state = LS_ERROR;
					l->err = emalloc(strlen(l->filename) + 43 + 1);
					sprintf(l->err, "%s: unexpected '.', not in a number literal\n", l->filename);
					result->type = LE_ERROR;
					return result;
				}
				break;
			case '(':
				switch(l->state) {
				case LS_WS:
					result->type = LE_LPAREN;
					result->len = strappend(result->lexeme, c, offset++, result->len);
					return result;
				default:
					l->pos--;
					l->state = LS_WS;
					return result;
				}
				break;
			case ')':
				switch(l->state) {
				case LS_WS:
					result->type = LE_RPAREN;
					result->len = strappend(result->lexeme, c, offset++, result->len);
					return result;
				default:
					l->pos--;
					l->state = LS_WS;
					return result;
				}
				break;
			case '\0':
				switch(l->state) {
				case LS_WS:
					free(result->lexeme);
					result->type = LE_EOF;
					break;
				default:
					l->pos--;
					l->state = LS_WS;
				}
				return result;
			default:
				l->state = LS_ERROR;
				l->err = emalloc(strlen(l->filename) + 16 + 1);
				sprintf(l->err, "%s: %c is garbage\n", l->filename, c);
				result->type = LE_ERROR;
				return result;
			}
		}
	}
	return result;
}

void
p_free(Parser *p)
{
	l_free(p->l);
	ast_free(p->ast);
	free(p->err);
	free(p);
}

Parser*
p_init(char *filename)
{
	Parser *p;

	p = emalloc(sizeof(Parser));
	p->ast = NULL;
	p->err = NULL;
	p->l = l_init(filename);
	return p;
}

/*
 * Shunting yard algorithm
 */
int
parse(Parser *p)
{
	size_t i;
	int found;
	Lexeme *le;
	Node *tmp;
	Stack *op_stack, *node_stack;
	Symbol *sym, *head;

	op_stack = node_stack = NULL;
	for(le = lex(p->l); le->type != LE_EOF && le->type != LE_ERROR; free(le), le = lex(p->l)) {
		switch(le->type){
		case LE_NUMBER:
			sym = num_alloc(atof(le->lexeme));
			node_stack = push(node_stack, ast_alloc(sym), NODE);
			free(le->lexeme);
			break;
		case LE_OPERATOR:
			sym = operator_alloc(le->lexeme[0]);
			free(le->lexeme);
			head = peek(op_stack);
			while(head != NULL &&
			      ! is_lparen(head) &&
			      precedence(head) >= precedence(sym)) {
				if(peek(op_stack) == NULL) goto err;
				tmp = ast_alloc(pop(&op_stack));

				/* Needs error checking */
				if(peek(node_stack) == NULL) goto err;
				ast_insert(tmp, pop(&node_stack));
				if(peek(node_stack) == NULL) goto err;
				ast_insert(tmp, pop(&node_stack));
				node_stack = push(node_stack, tmp, NODE);
				head = peek(op_stack);
			}
			op_stack = push(op_stack, sym, SYM);
			break;
		case LE_LPAREN:
			sym = lparen_alloc();
			free(le->lexeme);
			op_stack = push(op_stack, sym, SYM);
			break;
		case LE_RPAREN:
			while(! is_lparen(peek(op_stack))) {
				if(op_stack == NULL) {
					p->err = ecalloc(strlen(p->l->filename) + 26 + 1, sizeof(char));
					sprintf(p->err, "%s: unbalanced parenthesis\n", p->l->filename);
					return -1;
				}
				/* Error handling */
				if(peek(op_stack) == NULL) goto err;
				tmp = ast_alloc(pop(&op_stack));
				if(peek(node_stack) == NULL) goto err;
				ast_insert(tmp, pop(&node_stack));
				if(peek(node_stack) == NULL) goto err;
				ast_insert(tmp, pop(&node_stack));
				node_stack = push(node_stack, tmp, NODE);
			}
			symbol_free(pop(&op_stack)); /* Left paren, discarded */
			if(peek(op_stack) != NULL && is_function(peek(op_stack))) {
				tmp = ast_alloc(pop(&op_stack));
				ast_insert(tmp, pop(&node_stack));
				node_stack = push(node_stack, tmp, NODE);
			}
			free(le->lexeme);
			break;
		case LE_SYMBOL:
			if(strlen(le->lexeme) == 1) {
				sym = var_alloc(le->lexeme[0]);
				node_stack = push(node_stack, ast_alloc(sym), NODE);
				free(le->lexeme);
			} else {
				/* Throw error on unknown functions */
				found = 0;
				for(i = 0; i < KNOWN_FUNCS; i++) {
					if(strcmp(le->lexeme, known_funcs[i]) == 0) {
						sym = func_alloc(le->lexeme);
						op_stack = push(op_stack, sym, SYM);
						found = 1;
						break;
					}
				}
				if(! found) {
					stack_free(op_stack);
					stack_free(node_stack);
					p->err = ecalloc(strlen(p->l->filename) + strlen(le->lexeme) + 21 + 1, sizeof(char));
					sprintf(p->err, "%s: unknown function %s\n", p->l->filename, le->lexeme);
					free(le->lexeme);
					free(le);
					return -1;
				}
				found = 0;
			}
			break;
		default:
			stack_free(op_stack);
			stack_free(node_stack);
			p->err = ecalloc(strlen(p->l->filename) + strlen(le->lexeme) + 18 + 1, sizeof(char));
			sprintf(p->err, "%s: unknown symbol %s\n", p->l->filename, le->lexeme);
			free(le->lexeme);
			free(le);
			return -1;
		}
	}
	free(le);
	while(op_stack != NULL) {
		if(is_lparen(peek(op_stack))) {
			p->err = ecalloc(strlen(p->l->filename) + 26 + 1, sizeof(char));
			sprintf(p->err, "%s: unbalanced parenthesis\n", p->l->filename);
			stack_free(op_stack);
			stack_free(node_stack);
			return -1;
		}
		tmp = ast_alloc(pop(&op_stack));
		ast_insert(tmp, pop(&node_stack));
		ast_insert(tmp, pop(&node_stack));
		node_stack = push(node_stack, tmp, NODE);
	}
	p->ast = pop(&node_stack);
	if(stack_len(op_stack) > 0)
		goto err;
	else if(stack_len(node_stack) > 0)
		goto err;
	return 0;
err:
	p->err = ecalloc(strlen(p->l->filename) + 24 + 1, sizeof(char));
	sprintf(p->err, "%s: malformed expression\n", p->l->filename);
	stack_free(op_stack);
	stack_free(node_stack);

	return -1;
}

static void*
peek(Stack *s)
{
	return s == NULL ? NULL : s->data;
}

static void*
pop(Stack **s)
{
	void *data;
	Stack *old;

	data = NULL;
	if(*s != NULL) {
		data = (*s)->data;
		old = *s;
		*s = (*s)->next;
		free(old);
	}
	return data;
}

int
precedence(Symbol *s)
{
	if(s == NULL)
		return -1;
	switch(s->type) {
	case S_OP:
		switch(s->content->op){
		case '-':
		case '+':
			return 0;
		case '*':
		case '/':
			return 1;
		case '^':
			return 2;
		}
		break;
	case S_FUNC:
		return 2;
	default:
		return -1;
	}
	return -1;
}

static Stack*
push(Stack *s, void *data, enum stack_type type)
{
	Stack *new;

	new = stack_alloc(data, type);
	new->next = s;
	return new;
}

static Stack*
stack_alloc(void *data, enum stack_type type)
{
	Stack *s;

	s = emalloc(sizeof(Stack));
	s->data = data;
	s->type = type;
	s->next = NULL;
	return s;
}

static void
stack_free(Stack *s)
{
	if(s == NULL)
		return;
	while(s->next != NULL)
		if(s->type == SYM)
			symbol_free(pop(&s));
		else
			ast_free(pop(&s));
	if(s->type == SYM)
		symbol_free(s->data);
	else
		ast_free(pop(&s));
	free(s);
}

static int
stack_len(Stack *s)
{
	if(s == NULL)
		return 0;
	return 1 + stack_len(s->next);
}
