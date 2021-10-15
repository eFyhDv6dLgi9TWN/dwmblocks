PREFIX = /usr/local
CC = gcc

dwmblocks: dwmblocks.o
	${CC} $^ -lX11 -lpthread -o $@
	chmod 755 $@
dwmblocks.o: dwmblocks.c dwmblocks.h config.h
	${CC} -c $<
config.h: config.def.h
	cp $^ $@

all: dwmblocks
clean:
	rm -f *.o dwmblocks
install: dwmblocks
	mkdir -p ${PREFIX}/bin
	cp -f $^ ${PREFIX}/bin
uninstall:
	rm -f ${PREFIX}/bin/dwmblocks

.PHONY: all clean install uninstall
