

include ../../../makefile.env

CLASSES = AlgebraType.class AlgebraWriter.class ClassMethod.class 
CLASSES += Environment.class FinalPanel.class JavaWrapper.class 
CLASSES += OperatorSpecification.class Serializer.class SpecificationPanel.class 
CLASSES += TypeDescription.class TypeDescriptionPanel.class

ifeq ($(platform),win32)
S=;
else
S=:
endif

CP = $(SECONDO_BUILD_DIR)/Tools/Jni$S$(SECONDO_BUILD_DIR)/Javagui

.PHONY: all
all: $(CLASSES)

%.class: %.java
	javac -classpath $(CP) $<

.PHONY: clean
clean:
	rm -f *.class

