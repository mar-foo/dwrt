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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "dat.h"
#include "fns.h"

static void	usage(char*);

static void
usage(char *arg0)
{
	fprintf(stderr, "usage: %s [-l] variable\n", arg0);
}

int
main(int argc, char *argv[])
{
	int lflag, opt;
	Parser *p;
	Node *diff;

	opterr = lflag = 0;
	while((opt = getopt(argc, argv, "l")) != -1) {
		switch(opt) {
		case 'l':
			lflag = 1;
			break;
		default:
			usage(argv[0]);
		}
	}

	p = p_alloc(NULL);
	diff = NULL;

	if(parse(p) < 0) {
		fprintf(stderr, "%s", p->err);
		p_free(p);
		exit(1);
	}

	diff = ast_dwrt(p->ast, argv[optind][0]);
	if(lflag)
		ast_to_latex(diff);
	else
		ast_print(diff);

	printf("\n");
	ast_free(diff);
	p_free(p);

	return 0;
}
