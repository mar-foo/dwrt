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

#define LEN(x) (sizeof((x)) / sizeof((x)[0]))
#define KNOWN_FUNCS 8
#define KNOWN_OPERATORS 5

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

enum funcs {
	COS = 0x00,
	COSH,
	EXP,
	LOG,
	SIN,
	SINH,
	TAN,
	TANH
};

static struct func_to_bit {
	char *func;
	uint8_t bit;
} known_funcs[] = {
	{"cos", COS},
	{"cosh", COSH},
	{"exp", EXP},
	{"log", LOG},
	{"sin", SIN},
	{"sinh", SINH},
	{"tan", TAN},
	{"tanh", TANH}
};

enum operators {
	EXPT = 0x00,
	FRAC = 0x10,
	MUL = 0x20,
	SUB = 0x30,
	SUM = 0x40
};

static struct op_to_bit {
	char op;
	uint8_t bit;
} known_operators[] = {
	{'^', EXPT},
	{'/', FRAC},
	{'*', MUL},
	{'-', SUB},
	{'+', SUM}
};

#define IS_FUNC 0x0F;
#define IS_OP 0xF0;

typedef struct Lexeme Lexeme;
typedef struct Lexer Lexer;
typedef struct Node Node;
typedef struct Parser Parser;
typedef struct Symbol Symbol;

typedef Node* (*Derivative)(Node*, char);

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
		uint8_t func;
		double num;
		char var;
	} content;
};

