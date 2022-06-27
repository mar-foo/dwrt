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
	LE_SYMBOL,
	LE_RPAREN,
	LE_LPAREN
};

enum parse_state {
	P_EXPR,
	P_LITERAL
};

enum symbol_type {
	S_FUNC,
	S_VAR,
	S_NUM
};

typedef struct Function Function;
typedef struct Lexeme Lexeme;
typedef struct Lexer Lexer;
typedef struct Node Node;
typedef struct Parser Parser;
typedef struct Stack Stack;
typedef struct Symbol Symbol;

typedef double (*function)(double arg, ...);

struct Function {
	char *name;
	function *func;
};

struct Lexer {
	size_t len, line; /* data length, current line */
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
	Node *left, *right;
	Symbol *sym;
};

struct Parser {
	Lexer *l;
	enum parse_state state;
	char *err;
	Node *ast;
};

struct Symbol {
	enum symbol_type type;
	union {
		Function *func;
		char var;
		double num;
	} *content;
};
