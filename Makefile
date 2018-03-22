
CC=/usr/bin/gcc

CFLAGS=-I/Users/tee/anaconda3/envs/dev/include  -g
LDFLAGS=-L/Users/tee/anaconda3/envs/dev/lib -Xlinker -rpath -Xlinker /Users/tee/anaconda3/envs/dev/lib

.c.o:
	${CC} -c ${CFLAGS} $<
	
all: test

test: _demux.o _yuv2rgb.o
	${CC} $^ ${LDFLAGS} -lavcodec -lavformat -lavutil -lavfilter -lvpx -lswscale -lswresample
