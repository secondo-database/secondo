
JGUI=$(SECONDO_BUILD_DIR)/Javagui
CLASSPATH=.:$(JGUI):$(JGUI)/lib/java_cup_v10k.jar:$(JGUI)/lib/JLex_v1.2.6.jar

.PHONY: all
all: Projector.class

%.class: %.java
	javac -classpath $(CLASSPATH) $<


.PHONY: clean
clean:
	rm -f *.class
