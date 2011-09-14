
include ../../../makefile.env

BDBINCLUDE=$(BERKELEY_DB_DIR)/include
BDB_LIB_DIR=$(BERKELEY_DB_DIR)/lib

.PHONY: all
all: Config.ini app



Config.ini: $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini
	cp $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini ./Config.ini 
	sed -i -e "s#SecondoHome=.*#SecondoHome=$$PWD/nltemp#g" Config.ini


app: Example.o
	gcc -o app $(LDFLAGS) Example.o -L../lib/ -lsecondo  -L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB) -lfl -ly
	rm -rf nltemp
	mkdir nltemp


Example.o: Example.cpp
	$(CC) -c -o Example.o $(CCFLAGS) Example.cpp -I../include -I$(BDBINCLUDE) 

.PHONY:clean
clean:
	rm -f Config.ini app Example.o
	rm -rf nltemp
