CFLAGS = -Wall -Wextra -Werror -pedantic -ansi -D_POSIX_C_SOURCE=200809L
LDFLAGS =
TARG = dwrt
OBJ = parse.o util.o ast.o dwrt.o ast_nodes.o
SRC = $(OBJ:%.o=%.c)
PREFIX = /usr/local

ifeq (${DEBUG}, 1)
	CFLAGS += -ggdb
endif

ifeq (${COV}, 1)
	CFLAGS += -g -ftest-coverage -fprofile-arcs
endif

all: $(TARG)

ast_nodes.o: ast_nodes.c
	$(CC) $(CFLAGS) -lm -o $@ -c $^

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $^

cov: $(TARG)
	@test -f "dwrt.gcda" || echo "Run make COV=1 to generate coverage information"
	@test -d lcov || mkdir lcov
	gcov $(SRC) $(TARG)
	lcov -c -d . -o lcov/lcov.info
	genhtml -o lcov/ lcov/lcov.info

memcheck: $(TARG)
	valgrind --leak-check=full ./$(TARG) x

test: $(OBJ)
	$(MAKE) -C test CFLAGS="$(CFLAGS)" test

$(TARG): main.c $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(TARG) *.o *.gcov *.gcda *.gcno
	rm -rf lcov/*
	$(MAKE) -C test clean

tags:
	etags *.c *.h

check-syntax:
	$(CC) $(CFLAGS) -fsyntax-only $(CHK_SOURCES)

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f $(TARG) ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/$(TARG)

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/$(TARG)

.PHONY: all clean tags check-syntax install uninstall
