

include ../../../makefile.env
include ../../../Javagui/makefile.inc

CLASSES = AlgebraType.class AlgebraWriter.class ClassMethod.class 
CLASSES += Environment.class FinalPanel.class JavaWrapper.class 
CLASSES += OperatorSpecification.class Serializer.class SpecificationPanel.class 
CLASSES += TypeDescription.class TypeDescriptionPanel.class

ifeq ($(platform),win32)
S=;
else
S=:
endif

CP = $(SECONDO_BUILD_DIR)/Tools/Jni$S$(SECONDO_BUILD_DIR)/Javagui$S$(JARS)

.PHONY: all
all: sj $(CLASSES)

.PHONY: sj
sj:
	make -C $(SECONDO_BUILD_DIR)/Javagui/sj all

%.class: %.java
	javac -classpath $(CP) $<

.PHONY: clean
clean:
	rm -f *.class

