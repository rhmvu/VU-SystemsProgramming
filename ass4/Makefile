###################### DEFS

CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS = -ldl

###################### HELPERS

%.so : %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -nostdlib -fPIC $+ -o $@

%.o : %.c
	${CC} ${CFLAGS} $(LDFLAGS) -c $? -o $@

####################### TARGETS

# add your libraries to this line, as in 'libtest.so', make sure you have a 'libtest.c' as source
LIBS = libblank.so

#main lib containing program code


.PHONY : all clean distclean

all : client server ${LIBS}
	
client : audioclient.o audio.o libstream.so
	${CC} ${CFLAGS} -o $@ $+ $(LDFLAGS) -Wl,-rpath=$(shell pwd) -L./ -lstream

server : libstream.so audioserver.o audio.o 
	${CC} ${CFLAGS} -o $@ $+ $(LDFLAGS) -Wl,-rpath=$(shell pwd) -L./ -lstream

libstream.so : networking.o protocol.o
	${CC} -shared -o libstream.so networking.o protocol.o

networking.o : networking.c networking.h
	${CC} ${CFLAGS} -c -fPIC networking.c -o networking.o


protocol.o: protocol.c protocol.h
	${CC} ${CFLAGS} -c -fPIC protocol.c -o protocol.o

distclean : clean
	rm -f audioserver audioclient *.so
clean:
	rm -f $(OBJECTS) audioserver audioclient *.o *.so *~

