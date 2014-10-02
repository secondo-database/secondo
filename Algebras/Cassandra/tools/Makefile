all:	loadbalancer load


loadbalancer:	loadbalancer.cpp timer.h
	g++ loadbalancer.cpp -o loadbalancer -lpthread


load:	load1.cpp timer.h
	g++ load1.cpp -o load
