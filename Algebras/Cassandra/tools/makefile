.PHONY: all
all:	loadbalancer load tcpblackhole

.PHONY: clean
clean:
	rm -f loadbalancer load tcpblackhole

loadbalancer:	loadbalancer.cpp timer.h
	g++ loadbalancer.cpp -o loadbalancer -lpthread -O3

tcpblackhole: tcpblackhole.cpp
	g++  tcpblackhole.cpp -o tcpblackhole -lpthread -O3

load:	load1.cpp timer.h
	g++ load1.cpp -o load -O3
