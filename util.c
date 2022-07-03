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
#include <stdlib.h>

#include "dat.h"
#include "fns.h"

#define BUFSZ 512

void*
ecalloc(long nelem, size_t size)
{
	void *p;
	p = calloc(nelem, size);

	if(p == NULL) {
		perror("ecalloc");
		exit(1);
	}
	return p;
}

void*
emalloc(size_t size)
{
	void *p;
	p = malloc(size);

	if(p == NULL) {
		perror("emalloc");
		exit(1);
	}
	return p;
}

char*
readall(FILE *f)
{
	size_t bufsz, sz;
	char *data;

	bufsz = BUFSZ;
	data = ecalloc(bufsz, sizeof(char));
	while((sz = fread(data, sizeof(char), BUFSZ, f)) > 0) {
		if(sz == bufsz - 1) {
			bufsz *= 2;
			data = realloc(data, bufsz);
		}
	}
	data[bufsz - 1] = '\0';
	return data;
}

size_t
strappend(char *str, char c, size_t offset, size_t len)
{
	if(offset >= len) {
		len *= 2;
		str = realloc(str, len);
	}

	str[offset] = c;
	str[offset + 1] = '\0';
	return len;
}
