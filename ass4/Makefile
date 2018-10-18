###################### DEFS

CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS = -ldl

###################### HELPERS

#%.so : %.c
#	$(CC) $(CFLAGS) -shared -nostdlib -fPIC $+ -o $@

#%.o : %.c
#	${CC} ${CFLAGS} -c $? -o $@

####################### TARGETS

# add your libraries to this line, as in 'libtest.so', make sure you have a 'libtest.c' as source
LIBS = libblank.so

#main lib containing program code
libstream.so : networking.o protocol.o
	${CC} -shared -o libstream.so networking.o protocol.o

networking.o : networking.c networking.h
	echo "net"
	${CC} ${CFLAGS} -c -fPIC networking.c -o networking.o


protocol.o: protocol.c protocol.h
	echo "prot"
	${CC} ${CFLAGS} -c -fPIC protocol.c -o protocol.o

.PHONY : all clean distclean

all : client server ${LIBS}
	
client : client.o audio.o libstream.so
	${CC} ${CFLAGS} -o $@ $+ -Wl,-rpath=$(shell pwd) -L./ -lstream

server : libstream.so server.o audio.o 
	${CC} ${CFLAGS} -o $@ $+ -Wl,-rpath=$(shell pwd) -L./ -lstream

distclean : clean
	rm -f server client *.so
clean:
	rm -f $(OBJECTS) server client *.o *.so *~

