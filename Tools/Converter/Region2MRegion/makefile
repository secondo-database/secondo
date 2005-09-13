SBD := $(SECONDO_BUILD_DIR)

ifndef SBD 
	@echo "SECONDO_BUILD_DIR not defined."
	@echo "Ensure that this variable points to the Secondo's root directory."
	exit 1
endif



CP = $(SECONDO_BUILD_DIR)/Javagui:.

.PHONY:all
all: Gui  RegTransLinear.class 

RegTransLinear.class: Point.class ATransform.class RegTransLinear.java
	javac -classpath $(CP) RegTransLinear.java


Point.class: Point.java
	javac -classpath $(CP) Point.java

ATransform.class: ATransform.java Point.class
	javac -classpath $(CP) ATransform.java

Gui:
	@make -sC $(SBD)/Javagui all

.PHONY:clean
clean:
	rm -f *.class
