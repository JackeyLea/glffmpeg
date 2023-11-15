.POSIX:
.SUFFIXES:

all: example

example: glffmpeg_example/example
	cp glffmpeg_example/example example

glffmpeg_example/example: libglffmpeg.so
	$(MAKE) -C glffmpeg_example

libglffmpeg.so: glffmpeg/libglffmpeg.so
	cp glffmpeg/libglffmpeg.so libglffmpeg.so

glffmpeg/libglffmpeg.so:
	$(MAKE) -C glffmpeg
