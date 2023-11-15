include ../config.mak
CC = g++
#prefix = /usr/local
#exec_prefix = ${prefix}
#bindir = ${exec_prefix}/bin
#INSTALL = install
EXTRALIBS :=  \
		$(BUILD_ROOT)/libavformat/libavformat.a \
		$(BUILD_ROOT)/libavcodec/libavcodec.a \
		$(BUILD_ROOT)/libavutil/libavutil.a \
		$(EXTRALIBS)

CFLAGS+=-I$(SRC_PATH)/ -I$(SRC_PATH)/libavutil -I$(SRC_PATH)/libavformat -I$(SRC_PATH)/libavcodec

#BUILD_SHARED=yes 

OBJS :=  glffmpeg.obj

%.obj: %.cpp
	g++ $(CFLAGS) -c -o $@ $<

NAME=glffmpeg

$(NAME) : $(OBJS)
	rm -f $@
	$(CC)  -t -o lib$(NAME).so -shared -m32 $(OBJS) $(EXTRAOBJS) $(EXTRALIBS)

include ../common.mak
