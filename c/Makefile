CFLAGS ?= -O2 -Wall -Wextra -g

.PHONY = all clean

all: chess

clean:
	\rm chess

chess: main.c chess.c
	$(CC) $(CFLAGS) -o $@ $^
