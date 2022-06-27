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
