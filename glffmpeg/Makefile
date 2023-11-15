.POSIX:
.SUFFIXES:

include ../Makefile.config
LDLIBS = -lavformat -lavcodec -lavutil -lswscale
LDFLAGS = -L../
CFLAGS+=-I$(FFMPEG_SOURCE)

all: libglffmpeg.so

libglffmpeg.so: glffmpeg.o
	$(CXX) $(LDFLAGS) $(LDLIBS) -t -shared -o $@ $<

glffmpeg.o: glffmpeg.cpp
	$(CXX) $(CFLAGS) -c -fPIC -o $@ $<

