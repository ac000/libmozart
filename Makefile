LIBS=`pkg-config --libs gstreamer-0.10`
INCS=`pkg-config --cflags gstreamer-0.10`

libmozart: libmozart.o
	gcc -shared -Wl,-soname,libmozart.so.0 -o libmozart.so.0.0 libmozart.o -lc $(INCS) $(LIBS)

libmozart.o: libmozart.h libmozart.c player-operations.o nsleep.o
	gcc -fPIC -Wall -c libmozart.c $(INCS) $(LIBS)

player-operations.o: player-operations.h player-operations.c
	gcc -fPIC -Wall -c player-operations.c $(INCS) $(LIBS)

nsleep.o: nsleep.h nsleep.c
	gcc -fPIC -Wall -c nsleep.c

clean:
	rm libmozart.so.0.0 *.o

