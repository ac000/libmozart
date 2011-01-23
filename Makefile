LIBS=`pkg-config --libs gstreamer-0.10`
INCS=`pkg-config --cflags gstreamer-0.10`

libmozart: libmozart.o player-operations.o playlist-operations.o
	gcc -O2 -shared -Wl,-soname,libmozart.so.0 -o libmozart.so.0.0.0 libmozart.o player-operations.o playlist-operations.o -lc $(INCS) $(LIBS)

libmozart.o: libmozart.h libmozart.c
	gcc -O2 -fPIC -Wall -c libmozart.c -lm $(INCS) $(LIBS)

player-operations.o: player-operations.h player-operations.c
	gcc -O2 -fPIC -Wall -c player-operations.c $(INCS) $(LIBS)

playlist-operations.o: playlist-operations.h playlist-operations.c
	gcc -O2 -fPIC -Wall -c playlist-operations.c $(INCS) $(LIBS)

clean:
	rm libmozart.so.0.0.0 *.o

