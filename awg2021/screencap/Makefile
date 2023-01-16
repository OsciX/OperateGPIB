CC = gcc
CFLAGS = -Wall
LDFLAGS = -lgpib -lpng -lm
PROG = screencap

all: $(PROG)

$(PROG): src/$(PROG).c
	$(CC) -x c -g $(CFLAGS) $^ -o bin/$@ $(LDFLAGS)

clean: 
	rm -f *~ *.o $(PROG) core a.out