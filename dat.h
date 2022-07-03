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

enum lex_states {
	LS_ERROR,
	LS_NUMBER,
	LS_PAREN,
	LS_SYMBOL,
	LS_WS
};

enum lexeme_types {
	LE_ERROR,
	LE_EOF,
	LE_NUMBER,
	LE_OPERATOR,
	LE_SYMBOL,
	LE_RPAREN,
	LE_LPAREN
};

enum symbol_type {
	S_FUNC,
	S_LPAREN,
	S_NUM,
	S_OP,
	S_RPAREN,
	S_VAR
};

typedef struct Lexeme Lexeme;
typedef struct Lexer Lexer;
typedef struct Node Node;
typedef struct Parser Parser;
typedef struct Symbol Symbol;

struct Lexer {
	size_t len; /* data length */
	enum lex_states state; /* where was I? */
	char *filename, *err;
	char *data, *pos; /* contents of filename, current position */
};

struct Lexeme {
	enum lexeme_types type;
	size_t len;
	char *lexeme;
};

struct Node {
	Node *parent;
	Node *left, *right;
	Symbol *sym;
};

struct Parser {
	char *err;
	Lexer *l;
	Node *ast;
};

struct Symbol {
	enum symbol_type type;
	union {
		char *func;
		double num;
		char op;
		char var;
	} *content;
};
