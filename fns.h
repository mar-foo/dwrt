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

Node*	ast_alloc(Symbol*);
void	ast_free(Node*);
Node*	ast_copy(Node*);
void 	ast_insert(Node*, Node*);
void	ast_print(Node*);
Symbol*	func_alloc(char*);
Lexer*	l_init(char*);
void	l_free(Lexer*);
Lexeme*	lex(Lexer*);
Symbol*	lparen_alloc(void);
Symbol*	rparen_alloc(void);
Symbol*	num_alloc(double);
Symbol*	operator_alloc(char);
Symbol*	var_alloc(char);
void*	ecalloc(long, size_t);
void*	emalloc(size_t);
int	is_function(Symbol*);
int	is_lparen(Symbol*);
int	is_operator(Symbol*);
int	is_num(Symbol*);
int	num_equal(Symbol*, double);
void	p_free(Parser*);
Parser*	p_init(char*);
int	parse(Parser*);
char*	readall(FILE*);
size_t	strappend(char*, char, size_t, size_t);
void	symbol_free(Symbol*);
void	symbol_print(Symbol*);
