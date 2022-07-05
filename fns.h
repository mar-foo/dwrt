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

Node*	ast_alloc(Symbol*);
Node*	ast_copy(Node*);
Node*	ast_dwrt(Node*, char);
void	ast_free(Node*);
void 	ast_insert(Node*, Node*);
void	ast_print(Node*);
void	ast_to_latex(Node*);
void*	ecalloc(long, size_t);
void*	emalloc(size_t);
Symbol*	func_alloc(char*);
int	is_function(Symbol*);
int	is_lparen(Symbol*);
int	is_operator(Symbol*);
int	is_num(Symbol*);
Lexer*	l_alloc(char*);
void	l_free(Lexer*);
Lexeme*	lex(Lexer*);
Symbol*	lparen_alloc(void);
Symbol*	num_alloc(double);
int	num_equal(Symbol*, double);
Symbol*	operator_alloc(char);
Parser*	p_alloc(char*);
void	p_free(Parser*);
int	parse(Parser*);
int	precedence(Symbol*);
char*	readall(FILE*);
Symbol*	rparen_alloc(void);
size_t	strappend(char*, char, size_t, size_t);
void	symbol_free(Symbol*);
void	symbol_print(Symbol*);
Symbol*	var_alloc(char);
