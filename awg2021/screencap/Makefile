CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -lgpib
PROG = screencap

all: $(PROG)

$(PROG): src/$(PROG).c
	$(CC) -x c $(CFLAGS) $^ `pkg-config --cflags --libs MagickWand` -o bin/$@ $(LDFLAGS)

clean: 
	rm -f *~ *.o $(PROG) core a.out