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

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "../dat.h"
#include "../fns.h"

START_TEST(test_readall_short)
{
	char *buf;
	FILE *f;

	f = fopen("test_readall_short.txt", "r");
	ck_assert(f != NULL);

	buf = readall(f);
	ck_assert_str_eq(buf, "abcde\n");
	free(buf);
}
END_TEST

START_TEST(test_readall_long)
{
	char *buf;
	FILE *f;

	f = fopen("test_readall_long.txt", "r");
	ck_assert(f != NULL);

	buf = readall(f);
	ck_assert_str_eq(buf, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");
	free(buf);
}
END_TEST

START_TEST(test_strappend_no_realloc)
{
	char *buf;
	size_t len;

	len = 27;
	buf = calloc(len, sizeof(char));
	ck_assert(buf != NULL);

	buf = strcpy(buf, "abcdefg");

	strappend(buf, 'h', 7, len);
	ck_assert_str_eq(buf, "abcdefgh");

	free(buf);
}
END_TEST

START_TEST(test_strappend_with_realloc)
{
	char *buf;
	size_t len, offset;

	len = 3;
	offset = 0;
	buf = calloc(len + 1, sizeof(char));
	ck_assert(buf != NULL);

	len = strappend(buf, 'a', offset++, len);
	ck_assert_str_eq(buf, "a");
	len = strappend(buf, 'b', offset++, len);
	ck_assert_str_eq(buf, "ab");
	len = strappend(buf, 'c', offset++, len);
	ck_assert_str_eq(buf, "abc");
	len = strappend(buf, 'd', offset++, len);
	ck_assert_str_eq(buf, "abcd");

	free(buf);
}
END_TEST

Suite*
util_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("util");

	tc_core = tcase_create("core");

	tcase_add_test(tc_core, test_readall_short);
	tcase_add_test(tc_core, test_readall_long);
	tcase_add_test(tc_core, test_strappend_with_realloc);
	tcase_add_test(tc_core, test_strappend_no_realloc);
	suite_add_tcase(s, tc_core);

	return s;
}

int
main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = util_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
