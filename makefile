CC = gcc
CFLAGS = -std=c99 -Wpedantic -Wall -Wextra -O2
SRC = main.c lib/file.c lib/tools.c lib/inputhandler.c lib/etterna/header.c lib/quaver/header.c
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
	rm -f $(OBJ) $(OUTDIR) $(LOG)