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
static void	le_cleanup(Lexeme*);
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
					result->type = LE_SYMBOL;
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

static void
le_cleanup(Lexeme *lexeme)
{
		free(lexeme->lexeme);
		free(lexeme);
}

static Lexer*
l_init(char *filename)
{
	FILE *f;
	Lexer *l;

	l = emalloc(sizeof(Lexer));
	l->filename = filename,
	l->err = emalloc(0),
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
	return p;
}

void
parse(Parser *p)
{
	/* No compiler warnings for now */
	le_cleanup(NULL);
	lex(p->l);
	(void)p;
}

void
p_cleanup(Parser *p)
{
	l_cleanup(p->l);
	free(p->err);
	free(p);
}
