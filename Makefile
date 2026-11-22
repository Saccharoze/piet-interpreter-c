CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c99

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

piet: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

src/%.o: src/%.c $(wildcard src/*.h)
	$(CC) $(CFLAGS) -c -o $@ $<

test: piet
	tests/run.sh

clean:
	rm -f piet src/*.o
	rm -rf tests/build

.PHONY: test clean
