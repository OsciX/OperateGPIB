CXX = g++
CXXFLAGS = -w -Iinclude -mconsole
PROG = arb_plotter

all: $(PROG)

$(PROG): src/$(PROG).cpp
	$(CXX) -x c++ $(CXXFLAGS) $< -o bin/$@ $(OBJS) -Linclude -ltemp

obj/gpib488.o: include/gpib488.h

obj/tinyexpr.o: include/tinyexpr.h

clean: 
	rm -f *~ *.o $(PROG) core a.out