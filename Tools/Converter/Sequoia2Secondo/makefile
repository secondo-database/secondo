
CP = .:$(SECONDO_BUILD_DIR)/Javagui

.PHONY:all
all: SeqPoly.class 

%.class: %.java
	javac -classpath $(CP) $<

.PHONY:clean
clean:
	rm -f *.class
