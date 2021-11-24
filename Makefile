PREFIX = /usr/local
CC = gcc
CFLAGS = -Wall `pkgconf x11 --cflags` -O3

all: dwmblocks
dwmblocks: dwmblocks.o config.o
	${CC} -o $@ -lpthread `pkgconf x11 --libs` $^
	chmod 755 $@
%.o: %.c
	${CC} -c $< ${CFLAGS}
dwmblocks.o: dwmblocks.h config.h
config.o: dwmblocks.h config.h
config.c: config.def.c
	cp $< $@
config.h: config.def.h
	cp $< $@

clean:
	rm -f *.o dwmblocks
install: dwmblocks
	mkdir -p ${PREFIX}/bin
	cp -f $^ ${PREFIX}/bin
uninstall:
	rm -f ${PREFIX}/bin/dwmblocks

.PHONY: all clean install uninstall
