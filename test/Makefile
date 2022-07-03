TESTS = test_util test_ast test_parse test_dwrt
SRC = $(TESTS:%=%.c)
LDFLAGS += `pkg-config --libs check`
CFLAGS += `pkg-config --cflags check`

.PHONY: all clean

all: $(TESTS)

test_util: test_util.c ../util.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test_ast: test_ast.c ../ast.o ../util.o ../parse.o

test_parse: test_parse.c ../parse.o ../util.o ../ast.o

test_dwrt: test_dwrt.c ../ast.o ../util.o ../parse.o

test: $(TESTS)
	for t in $(TESTS); do ./$$t ; done

clean:
	rm -f $(TESTS) *.gcno *.gcda *.gcov
