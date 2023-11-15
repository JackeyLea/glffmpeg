.POSIX:
.SUFFIXES:

include ../Makefile.config

CXXFLAGS = -I../glffmpeg
LDLIBS = -lglffmpeg -lGLU -lGL -lX11
LDFLAGS = -L.. -Wl,-rpath ..

# Default rule
example: glffmpeg_example.o
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $<

%.o: %.cpp
	g++ $(CXXFLAGS) -c -o $@ $<


