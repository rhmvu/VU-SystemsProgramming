pingclient1: pingclient1.o
	gcc -o pingclient1 pingclient1.o
pingclient2: pingclient2.o
	gcc -o pingclient2 pingclient2.o pingclient1.o
pingclient3: pingclient3.o
	gcc -o pingclient3 pingclient1.o pingclient2.o pingclient3.o
	

pingclient1.o: pingclient1.c pingclient1.h
	gcc -c -Wall pingclient1.c
pingclient2.o: pingclient2.c pingclient2.h pingclient1.h pingclient1.c
	gcc -c -Wall pingclient2.c
pingclient3.o: pingclient 3.c pingclient3.h pingclient2.h pingclient2.c pingclient.h pingclient.c
	gcc -c -Wall pingclient3.c

depend:
	makedepend pingclient1.c pingclient2.c pingclient3.c



# DO NOT DELETE

pingclient1.o: /usr/include/stdio.h /usr/include/features.h
pingclient1.o: /usr/include/stdc-predef.h /usr/include/libio.h
pingclient1.o: /usr/include/_G_config.h /usr/include/wchar.h
pingclient1.o: /usr/include/stdlib.h /usr/include/alloca.h
pingclient1.o: /usr/include/unistd.h /usr/include/getopt.h
pingclient1.o: /usr/include/netinet/in.h /usr/include/stdint.h
pingclient1.o: /usr/include/endian.h /usr/include/arpa/inet.h
pingclient1.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
pingclient1.o: /usr/include/time.h /usr/include/xlocale.h
pingclient1.o: /usr/include/string.h
pingclient2.o: /usr/include/stdio.h /usr/include/features.h
pingclient2.o: /usr/include/stdc-predef.h /usr/include/libio.h
pingclient2.o: /usr/include/_G_config.h /usr/include/wchar.h
pingclient2.o: /usr/include/stdlib.h /usr/include/alloca.h
pingclient2.o: /usr/include/unistd.h /usr/include/getopt.h
pingclient2.o: /usr/include/netinet/in.h /usr/include/stdint.h
pingclient2.o: /usr/include/endian.h /usr/include/arpa/inet.h
pingclient2.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
pingclient2.o: /usr/include/time.h /usr/include/xlocale.h
pingclient2.o: /usr/include/string.h pingclient1.h
pingclient3.o: /usr/include/stdio.h /usr/include/features.h
pingclient3.o: /usr/include/stdc-predef.h /usr/include/libio.h
pingclient3.o: /usr/include/_G_config.h /usr/include/wchar.h
pingclient3.o: /usr/include/stdlib.h /usr/include/alloca.h
pingclient3.o: /usr/include/unistd.h /usr/include/getopt.h
pingclient3.o: /usr/include/netinet/in.h /usr/include/stdint.h
pingclient3.o: /usr/include/endian.h /usr/include/arpa/inet.h
pingclient3.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
pingclient3.o: /usr/include/time.h /usr/include/xlocale.h
pingclient3.o: /usr/include/string.h pingclient1.h pingclient2.h
