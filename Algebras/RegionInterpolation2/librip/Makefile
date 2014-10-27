SUITE=STABLE
#SUITE=EXPERIMENTAL
export SUITE

all:
ifeq ($(SUITE),EXPERIMENTAL)
	make -C devel
else
	make -C lib
endif
	make -C example

install:
	cp build/librip.so /usr/local/lib
	cp include/librip.h /usr/local/include

clean:
	make -C lib clean
	make -C devel clean
	make -C example clean
	rm -f build/*
