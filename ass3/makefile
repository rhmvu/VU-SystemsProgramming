all:
	make pingclient1;\
	make pingclient2;\
	make pingclient3;\
	make pingserver;\

pingclient1: pingclient1.o libpingutils.so
	gcc -o pingclient1 pingclient1.o -L. -lpingutils
pingclient2: pingclient2.o libpingutils.so
	gcc -o pingclient2 pingclient2.o -L. -lpingutils
pingclient3: pingclient3.o libpingutils.so
	gcc -o pingclient3 pingclient3.o -L. -lpingutils
pingserver: pingserver.c
	gcc -Wall -O -o pingserver pingserver.c



pingclient1.o: pingclient1.c
	gcc -c -fPIC -Wall pingclient1.c
pingclient2.o: pingclient2.c
	gcc -c -fPIC -Wall pingclient2.c
pingclient3.o: pingclient3.c
	gcc -c -fPIC -Wall pingclient3.c
pingutils.o: pingutils.c pingutils.h
	gcc -c -fPIC -Wall pingutils.c
pingutils2.o:  pingutils2.c pingutils2.h
	gcc -c -fPIC -Wall pingutils2.c


libpingutils.so: pingutils.o pingutils2.o
	gcc -shared -o libpingutils.so pingutils.o pingutils2.o &&\
	export LD_LIBRARY_PATH=$PWD
depend:
	makedepend pingclient1.c pingclient2.c pingclient3.c pingutils.c pingutils2.c pingserver.c

clean:
	rm -rf *.o
	rm -rf *.so