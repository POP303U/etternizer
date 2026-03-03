CC = gcc
CFLAGS = -std=c99 -lm -Wpedantic -Wall -Wextra -O2
SRC = main.c lib/file.c lib/tools.c lib/input.c lib/etterna/header.c lib/quaver/parser.c lib/quaver/notes.c lib/etterna/export.c
OUTDIR = out
OUT = $(OUTDIR)/etternizer
OBJ = $(SRC:.c=.o)
LOG = $(SRC:.log)

all: $(OUT)

# rule to create dir if it doesn't exist
$(OUTDIR):
	mkdir -p $(OUTDIR)

# depend on outdir to build
$(OUT): | $(OUTDIR)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(STATIC)

run:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(STATIC)
	./$(OUT) $(FILE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUTDIR)/etternizer $(LOG)