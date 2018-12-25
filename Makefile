INCLUDES = -Iinclude -I/usr/local/include
LIBS = -Llib -lportsf -lm -L/usr/local/lib -lgsl
CC = gcc

noisemachine: noisemachine.c lib/libportsf.a
	$(CC) -o noisemachine noisemachine.c $(INCLUDES) $(LIBS)
