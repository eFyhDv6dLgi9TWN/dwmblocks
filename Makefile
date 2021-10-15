PREFIX = /usr/local
CC = gcc

dwmblocks: dwmblocks.o
	${CC} -o $@ -lpthread `pkgconf x11 --cflags --libs` $^
	chmod 755 $@
dwmblocks.o: dwmblocks.c config.h
	${CC} -c $<
config.h:
	cp config.def.h $@

all: dwmblocks
clean:
	rm -f *.o dwmblocks
install: dwmblocks
	mkdir -p ${PREFIX}/bin
	cp -f $^ ${PREFIX}/bin
uninstall:
	rm -f ${PREFIX}/bin/dwmblocks

.PHONY: all clean install uninstall
