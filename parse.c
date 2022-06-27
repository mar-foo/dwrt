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

#define LEXEME_MINSZ 10


typedef struct Stack Stack;
struct Stack {
	Symbol *sym;
	Stack *next;
};

int precedence_table[] = { 0, 1, 2, 3 };

static Stack*	alloc_stack(Symbol*);
static void	cleanup_stack(Stack*);
static char	l_getc(Lexer*);
static Lexer*	l_init(char*);
static Lexeme*	lex(Lexer*);
static Symbol*	peek(Stack*);
static int	precedence(Symbol*);
static Symbol*	pop(Stack**);
static Stack*	push(Stack*, Symbol*);

static Stack*
alloc_stack(Symbol *sym)
{
	Stack *s;

	s = emalloc(sizeof(Stack));
	s->sym = sym;
	s->next = NULL;
	return s;
}

static void
cleanup_stack(Stack *s)
{
	while(s->next != NULL)
		symbol_cleanup(pop(&s));
	symbol_cleanup(s->sym);
	free(s);
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
	offset = 0;
	while((c = l_getc(l)) != '\0') {
		if(isspace(c)) {
			if(c == '\n')
				l->line++;
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
					l->err = emalloc(strlen(l->filename) + 45 + 1);
					sprintf(l->err, "%s:%ld unexpected '.', not in a number literal\n", l->filename, l->line);
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
			case EOF:
				result->type = LE_EOF;
				return result;
			default:
				l->state = LS_ERROR;
				l->err = emalloc(strlen(l->filename) + 19 + 1);
				sprintf(l->err, "%s:%ld %c is garbage\n", l->filename, l->line, c);
				result->type = LE_ERROR;
				return result;
			}
		}
	}
	/* Unreachable */
	result->type = LE_EOF;
	return result;
}

static Lexer*
l_init(char *filename)
{
	FILE *f;
	Lexer *l;

	l = emalloc(sizeof(Lexer));
	l->filename = filename,
	l->err = NULL,
	l->line = 1,
	l->state = LS_WS;

	if((f = fopen(filename, "r")) == NULL)
		return NULL;
	l->pos = l->data = readall(f);

	fclose(f);
	return l;
}

void
l_cleanup(Lexer *lex)
{
	free(lex->data);
	free(lex->err);
	free(lex);
}

static char
l_getc(Lexer *l)
{
	return *(l->pos++);
}

void
p_cleanup(Parser *p)
{
	l_cleanup(p->l);
	ast_cleanup(p->ast);
	free(p->err);
	free(p);
}

Parser*
p_init(char *filename)
{
	Parser *p;

	p = emalloc(sizeof(Parser));
	p->err = NULL;
	p->l = l_init(filename);
	p->ast = alloc_node(alloc_var('='));
	p->state = P_INIT;
	p->parens = 0;
	return p;
}

/*
 * Shunting yard algorithm
 */
int
parse(Parser *p)
{
	Lexeme *le;
	Stack *op_stack;
	Symbol *sym, *first;

	for(le = lex(p->l); le->type != LE_EOF && le->type != LE_ERROR; free(le), le = lex(p->l)) {
		switch(le->type){
		case LE_NUMBER:
			break;
		case LE_OPERATOR:
			sym = alloc_func(le->lexeme);
			first = peek(op_stack);
			while(! is_lparen(first) &&
			      is_operator(first) &&
			      precedence(first) > precedence(sym))
				first = pop(&op_stack); /* Add it in the tree */
			push(op_stack, sym);
			break;
		case LE_LPAREN:
			sym = alloc_lparen();
			free(le->lexeme);
			push(op_stack, sym);
		case LE_RPAREN:
			while(! is_lparen(peek(op_stack))) {
				if(op_stack == NULL) {
					p->err = ecalloc(strlen(p->l->filename) + 29 + 1, sizeof(char));
					sprintf(p->err, "%s:%ld unbalanced parenthesis\n", p->l->filename, p->l->line);
					return -1;
				}
				first = pop(&op_stack); /* Add it in the tree */
			}
			pop(&op_stack); /* Left paren, discarded */
			if(is_function(peek(op_stack)))
				first = pop(&op_stack); /* Add it in the tree */
		case LE_EOF:
			break;
		default:
			cleanup_stack(op_stack);
			return -1;
		}
	}
	while(op_stack != NULL)
		first = pop(&op_stack); /* Add it in the tree */

	return 0;
}

static Symbol*
peek(Stack *s)
{
	return s == NULL ? NULL : s->sym;
}

static Symbol*
pop(Stack **s)
{
	Symbol *sym;
	Stack *old;

	sym = (*s)->sym;
	old = *s;
	*s = (*s)->next;
	free(old);
	return sym;
}

static int
precedence(Symbol *s)
{
	if(s == NULL)
		return -1;
	switch(s->type) {
	case S_OP:
		switch(s->content->op){
		case '-':
		case '+':
			return precedence_table[O_ADD];
		case '*':
		case '/':
			return precedence_table[O_MUL];
		}
		break;
	case S_FUNC:
		return precedence_table[O_FUNC];
	default:
		return -1;
	}
	return -1;
}

static Stack*
push(Stack *s, Symbol *sym)
{
	Stack *new;

	new = alloc_stack(sym);
	new->next = s;
	return new;
}
