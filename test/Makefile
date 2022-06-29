TESTS = test_util test_ast test_parse
SRC = $(TESTS:%=%.c)
LDFLAGS = `pkg-config --libs check`
CFLAGS = `pkg-config --cflags check`

.PHONY: all clean

all: $(TESTS)

test_util: test_util.c ../util.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test_ast: test_ast.c ../ast.o ../util.o

test_parse: test_parse.c ../parse.o ../util.o ../ast.o

test: $(TESTS)
	for t in $(TESTS); do ./$$t ; done

clean:
	rm $(TESTS)
