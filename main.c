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

#include <stdio.h>
#include <stdlib.h>

#include "dat.h"
#include "fns.h"

static void	usage(char*);

static void
usage(char *arg0)
{
	fprintf(stderr, "usage: %s variable\n", arg0);
}

int
main(int argc, char *argv[])
{
	Parser *p;
	Node *diff;

	if(argc != 2) {
		usage(argv[0]);
		exit(1);
	}

	p = p_init(NULL);
	diff = NULL;

	if(parse(p) < 0) {
		fprintf(stderr, "%s", p->err);
		p_free(p);
		exit(1);
	}

	diff = ast_derive(p->ast, argv[1][0]);
	ast_to_latex(diff);
	printf("\n");
	ast_free(diff);
	p_free(p);

	return 0;
}
