all:
	make -C lib
	make -C example

install:
	cp lib/librip.so /usr/local/lib
	cp include/librip.h /usr/local/include

clean:
	make -C lib clean
	make -C example clean
