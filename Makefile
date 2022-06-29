CFLAGS = -Wall -Wextra -Werror -pedantic -ansi
LDFLAGS = -lm
TARG = derive
OBJ = parse.o util.o ast.o

ifeq (${DEBUG}, 1)
	CFLAGS += -ggdb
endif

ifeq (${COV}, 1)
	CFLAGS += -g -ftest-coverage -fprofile-arcs
endif

.PHONY: all clean tags check-syntax

all: $(TARG)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $^

cov: $(TARG)
	@test -f "derive.gcda" || echo "Run make COV=1 to generate coverage information"
	gcov $(OBJ) $(TARG)

memcheck: $(TARG)
	valgrind --leak-check=full ./$(TARG) x

test: $(OBJ) memcheck
	$(MAKE) -C test CFLAGS="$(CFLAGS)" test

$(TARG): $(TARG).c $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm $(TARG) *.o *.gcov *.gcda *.gcno
	$(MAKE) -C test clean

tags:
	etags *.c *.h

check-syntax:
	$(CC) $(CFLAGS) -fsyntax-only $(CHK_SOURCES)
