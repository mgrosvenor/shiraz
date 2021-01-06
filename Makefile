CFLAGS= -Wall 
LIBS=
OUTDIR=bin


.PHONY: all

all: debug

release: CFLAGS += -O3 -DNDEBUG
release: demo

debug: CFLAGS += -Werror -g -pedantic -std=c99 -Wextra -fsanitize=address -fno-omit-frame-pointer
debug: test demo

test: test.c shiraz.h
	mkdir -p $(OUTDIR)
	$(CC) -o $(OUTDIR)/$@ test.c $(CFLAGS) $(LIBS)

demo: demo.c shiraz.h
	mkdir -p $(OUTDIR)
	$(CC) -o $(OUTDIR)/$@ demo.c $(CFLAGS) $(LIBS)
	

.PHONY: clean

clean:
	rm -rf $(OUTDIR)/*
