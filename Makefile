CXX = g++
LD = g++

OBJS = $(CXXFILES:.cc=.o)

CXXFLAGS = `pkg-config --cflags sdl gl glu` -Wall -g -O2 -std=c++0x
LIBS = `pkg-config --libs sdl gl glu`

CXXFILES = main-sdl.cc game.cc grid.cc gfx-sdl.cc

TARGET = demo

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LIBS)

depend: .depend

.depend: $(CXXFILES)
	rm -f .depend
	$(CXX) $(CXXFLAGS) -MM $^ > .depend;

clean:
	rm -f *o $(TARGET)

include .depend

.PHONY: all clean depend
