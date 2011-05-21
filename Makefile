LIBS=`pkg-config --libs gstreamer-0.10`
INCS=`pkg-config --cflags gstreamer-0.10`

libmozart: libmozart.o player-operations.o playlist-operations.o
	gcc -g -shared -Wl,-soname,libmozart.so.0 -o libmozart.so.0.0.0 libmozart.o player-operations.o playlist-operations.o -lc $(INCS) $(LIBS)

libmozart.o: libmozart.h libmozart.c
	gcc -g -fPIC -Wall -c libmozart.c $(INCS)

player-operations.o: player-operations.h player-operations.c
	gcc -g -fPIC -Wall -c player-operations.c $(INCS)

playlist-operations.o: playlist-operations.h playlist-operations.c
	gcc -g -fPIC -Wall -c playlist-operations.c $(INCS)

clean:
	rm libmozart.so.0.0.0 *.o

