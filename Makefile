LD = $(CC)
CFLAGS = -std=c99 -pedantic-errors -fextended-identifiers -Wall -W -Wstrict-prototypes -O3
LDLIBS = -lglut -lGLU -lGL -lm
BIN = toy-train

$(BIN): $(patsubst %.c,%.o,$(wildcard *.c))
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f *.o $(BIN)

.PHONY: run
run:	$(BIN)
	./$<
