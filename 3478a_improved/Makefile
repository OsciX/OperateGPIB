CXX = g++
CXXFLAGS = -w -Iinclude -mconsole
PROG = 3478a_improved

all: $(PROG)

$(PROG): src/$(PROG).cpp
	$(CXX) -x c++ $(CXXFLAGS) $< -o bin/$@ $(OBJS) -Linclude -ltemp

obj/gpib488.o: include/gpib488.h

clean: 
	rm -f *~ *.o $(PROG) core a.out