CC = gcc
CFLAGS = -Wall -Iinclude -mconsole
PROG = acq_test


all: $(PROG)

acq_test: src/acq_test.c
	$(CC) -x c $(CFLAGS) $< -o bin/$@ $(OBJS) -Linclude -ltemp

obj/gpib488.o: include/gpib488.h

clean: 
	rm -f *~ *.o $(PROG) core a.out