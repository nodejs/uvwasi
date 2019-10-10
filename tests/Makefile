MAIN=main.o
TESTS=test_uv_mapping.c test_fd_table.o
OUTDIR=./out
PARENTDIR=..
CC=gcc
CFLAGS+=-Wall -Wsign-compare -I$(PARENTDIR)/include
LIBS=-luv -lcheck
TEST_OBJ=$(patsubst %.c,%.o, $(patsubst %,$(OUTDIR)/%,$(TESTS)))
UVWASI_OBJ=$(wildcard $(PARENTDIR)/out/obj/*.o)
MKDIRP = mkdir -p

$(OUTDIR)/%.o: %.c | out
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTDIR)/main: main.c $(TEST_OBJ) | out
	$(CC) $(UVWASI_OBJ) $(TEST_OBJ) -o $@ $< $(CFLAGS) $(LIBS)

.PHONE: check
check: $(OUTDIR)/main
	@$(OUTDIR)/main

.PHONY: clean

clean:
	rm -rf $(OUTDIR)

out:
	@$(MKDIRP) $(OUTDIR)
