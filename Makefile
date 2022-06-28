CFLAGS = -Wall -Wextra -Werror -pedantic -ansi
LDFLAGS = -lm
TARG = derive
OBJ = parse.o util.o ast.o

ifeq (${DEBUG}, 1)
	CFLAGS += -ggdb
endif

.PHONY: all clean tags check-syntax

all: $(TARG)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $^

test: $(OBJ)
	$(MAKE) -C test

$(TARG): $(TARG).c $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm $(TARG) *.o

tags:
	etags *.c *.h

check-syntax:
	$(CC) $(CFLAGS) -fsyntax-only $(CHK_SOURCES)
