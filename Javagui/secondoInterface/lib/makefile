CLASSES := $(wildcard java_cup10/*.class) $(wildcard java_cup10/runtime/*.class)

.PHONY: all
all: java_cup10 java_cup_v10k.jar

.PHONY: java_cup10
java_cup10:
	$(MAKE) -C $@ all

java_cup_v10k.jar: $(CLASSES) | java_cup10
	jar -cf $@ java_cup10

.PHONY: clean
clean: 
	$(MAKE) -C java_cup10 clean
	rm -f java_cup_v10k.jar
