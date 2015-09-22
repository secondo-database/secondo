.PHONY: all
all:	loadbalancer load

.PHONY: clean
clean:
	rm loadbalancer load

loadbalancer:	loadbalancer.cpp timer.h
	g++ loadbalancer.cpp -o loadbalancer -lpthread -O3


load:	load1.cpp timer.h
	g++ load1.cpp -o load -O3
