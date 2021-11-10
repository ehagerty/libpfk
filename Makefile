CFLAGS=-fPIC -g -I./include
SONAME=liblpfk.so.1

.PHONY:	all doc clean

all:	liblpfk.so lpfktest lpfklife lpfkbinclock
	ldconfig -n .

doc:	Doxyfile src/liblpfk.c include/liblpfk.h
	doxygen

clean:
	-rm -f lpfktest lpfklife lpfkbinclock liblpfk.so*
	-rm -f src/*.o test/*.o
	-rm -f src/*~ test/*~ *~

liblpfk.so:	src/liblpfk.o
	$(CC) -shared -Wl,-soname,$(SONAME) -o $@ $<

lpfktest:	test/lpfktest.o
	$(CC) -o $@ $< -L. -llpfk

lpfklife:	test/lpfklife.o
	$(CC) -o $@ $< -L. -llpfk

lpfkbinclock:	test/lpfkbinclock.o
	$(CC) -o $@ $< -L. -llpfk

src/liblpfk.o:		include/liblpfk.h
test/lpfktest.o:	include/liblpfk.h
test/lpfklife.o:	include/liblpfk.h

