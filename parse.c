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

static char	l_getc(Lexer*);
static Lexer*	l_init(char*);
static Lexeme*	lex(Lexer*);

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

int
parse(Parser *p)
{
	Lexeme *le;
	Node *root;
	Symbol *sym;

	root = p->ast;
	le = lex(p->l);
	switch(le->type) {
	case LE_SYMBOL:
		if(strlen(le->lexeme) == 1) {
			sym = alloc_var(le->lexeme[0]);
			ast_insert(p->ast, alloc_node(sym));
			if(p->ast->right == NULL)
				return parse(p);
		} else {
			Node *dummy_node;
			sym = alloc_func(le->lexeme);
			dummy_node = alloc_node(alloc_num(0));

			p->ast->left = dummy_node;
			p->ast = ast_insert(p->ast, alloc_node(sym));
			return parse(p);
		}
		break;
	case LE_NUMBER:
		sym = alloc_num(atof(le->lexeme));
		p->ast = ast_insert(p->ast, alloc_node(sym));
		break;
	case LE_OPERATOR:
		sym = alloc_func(le->lexeme);
		p->ast = ast_insert_above(p->ast->left, alloc_node(sym));
		return parse(p);
		break;
	case LE_LPAREN:
		p->parens++;
		while(p->parens != 0)
			return parse(p);
		break;
	case LE_RPAREN:
		p->parens--;
	case LE_EOF:
		break;
	case LE_ERROR:
		fprintf(stderr, "%s", p->l->err);
		p->ast = root;
		return -1;
	}
	free(le);

	p->ast = root;
	return 0;
}

void
p_cleanup(Parser *p)
{
	l_cleanup(p->l);
	ast_cleanup(p->ast);
	free(p->err);
	free(p);
}
