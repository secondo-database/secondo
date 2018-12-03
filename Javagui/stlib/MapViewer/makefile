# file: stlib/MapViewer/makefile



JAVAC := javac
CLASSPATH := ./src
LIBPATH=./lib
JARS := $(LIBPATH)/MovingObjectsLibrary.jar
JARS := $(JARS):$(LIBPATH)/jxmapviewer2-2.3.jar
JARS := $(JARS):$(LIBPATH)/commons-logging-1.1.1.jar

CLASSPATH := $(CLASSPATH):$(JARS):.
JAVAFILES =$(shell find -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))

.PHONY:all
all: $(CLASSFILES);

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) -d bin $<


.PHONY:jar
jar:	makedir	all
	@cp lib/*.jar bin/.
	@cd bin;find . -name "*jar" -exec jar -xf {} \; ; cd ..
	@find bin -name "*jar" -exec rm -f {} \;
	@echo "Class-Path: ." > Manifest.txt
	@echo "Main-Class: mapviewer.MapViewer" >> Manifest.txt
	jar -cmf Manifest.txt jar/MapViewer.jar -C bin/ .
	rm -f Manifest.txt
	rm -frd bin/*
	rmdir bin
	@chmod ugo+x jar/MapViewer.jar

.PHONY:makedir
makedir:
	mkdir bin

.PHONY:clean
clean:
	find . -name '*.class' | xargs rm -rf
	rm -f jar/MapViewer.jar
	rm -frd bin/*
