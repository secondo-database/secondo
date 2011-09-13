
#include ../../../makefile.env

BDBINCLUDE=$(BERKELEY_DB_DIR)/include
BDB_LIB_DIR=$(BERKELEY_DB_DIR)/lib

.PHONY: all
all: config app


config: SecondoConfig.ini

SecondoConfig.ini: $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini
	cp $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini .
	sed -i -e "s#SecondoHome=.*#SecondoHome=$$PWD/nltemp#g" SecondoConfig.ini


app: Example.o
	gcc -o app Example.o -L../lib/ -lsecondo  -L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB) -lfl -ly
	rm -rf nltemp
	mkdir nltemp


Example.o: Example.cpp
	gcc -c -o Example.o Example.cpp -I../include -I$(BDBINCLUDE) 

.PHONY:clean
clean:
	rm -f SecondoConfig.ini app Example.o
	rm -rf nltemp
