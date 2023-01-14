CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -lgpib -lm
PROG = acquire

all: $(PROG)

$(PROG): src/$(PROG).c
	$(CC) -x c $(CFLAGS) $^ -o bin/$@ $(LDFLAGS)

clean: 
	rm -f *~ *.o $(PROG) core a.out