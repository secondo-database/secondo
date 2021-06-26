.PHONY: all
all:	loadbalancer load tcpblackhole
	$(MAKE) -C queryexecutor

.PHONY: clean
clean:
	rm -f loadbalancer load tcpblackhole
	$(MAKE) -C queryexecutor clean

loadbalancer:	loadbalancer.cpp timer.h
	g++ loadbalancer.cpp -o loadbalancer -lpthread -O3 $(CPPSTDOPTION)

tcpblackhole: tcpblackhole.cpp
	g++ tcpblackhole.cpp -o tcpblackhole -lpthread -O3 $(CPPSTDOPTION)

load:	load1.cpp timer.h
	g++ load1.cpp -o load -O3 $(CPPSTDOPTION)
