SUITE=STABLE
#SUITE=EXPERIMENTAL
export SUITE

all: build
ifeq ($(SUITE),EXPERIMENTAL)
	$(MAKE) -C devel
else
	$(MAKE) -C lib
endif
	$(MAKE) -C example

build:
	mkdir -p build

install:
	cp build/librip.so /usr/local/lib
	cp include/librip.h /usr/local/include

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C devel clean
	$(MAKE) -C example clean
	rm -rf build
