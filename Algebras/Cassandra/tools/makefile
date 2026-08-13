.PHONY: all
all:	loadbalancer load tcpblackhole
	$(MAKE) -C queryexecutor

.PHONY: clean
clean:
	rm -f loadbalancer load tcpblackhole
	$(MAKE) -C queryexecutor clean

loadbalancer:	loadbalancer.cpp timer.h
	$(CPPC) loadbalancer.cpp -o loadbalancer -lpthread -O3 $(CPPSTDOPTION)

tcpblackhole: tcpblackhole.cpp
	$(CPPC) tcpblackhole.cpp -o tcpblackhole -lpthread -O3 $(CPPSTDOPTION)

load:	load1.cpp timer.h
	$(CPPC) load1.cpp -o load -O3 $(CPPSTDOPTION)
