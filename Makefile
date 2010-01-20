LIBS=`pkg-config --libs gstreamer-0.10`
INCS=`pkg-config --cflags gstreamer-0.10`

libmozart: libmozart.o player-operations.o 
	gcc -shared -Wl,-soname,libmozart.so.0 -o libmozart.so.0.0 libmozart.o player-operations.o -lc $(INCS) $(LIBS)

libmozart.o: libmozart.h libmozart.c
	gcc -fPIC -Wall -c libmozart.c -lm $(INCS) $(LIBS)

player-operations.o: player-operations.h player-operations.c
	gcc -fPIC -Wall -c player-operations.c $(INCS) $(LIBS)

clean:
	rm libmozart.so.0.0 *.o

