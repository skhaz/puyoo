CC = gcc
LD = gcc

OBJS = $(CFILES:.c=.o)

CFLAGS = `pkg-config --cflags sdl gl glu` -Wall -g -O2
LIBS = `pkg-config --libs sdl gl glu`

CFILES = main-sdl.c game.c grid.c gfx-sdl.c

TARGET = demo

.c.o:
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LIBS)

depend: .depend

.depend: $(CFILES)
	rm -f .depend
	$(CC) $(CFLAGS) -MM $^ > .depend;

clean:
	rm -f *o $(TARGET)

include .depend

.PHONY: all clean depend
