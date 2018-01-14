
all:
	gcc _demux.c -I/home/tee/anaconda3/envs/dev/include/ -L/home/tee/anaconda3/envs/dev/lib -lavcodec -lavformat -lavutil -lavfilter -lvpx -lswscale -lswresample -g
