

.PHONY: all

all: 
	$(MAKE) -C java_cup10 all
	jar -cf java_cup_v10k.jar java_cup10


.PHONY: clean

clean: 
	$(MAKE) -C java_cup10 clean
	rm -f java_cup_v10k.jar
