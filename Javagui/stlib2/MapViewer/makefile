# file: stlib/MapViewer/makefile



JAVAC := javac
CLASSPATH := ./src
LIBPATH=./lib
JARS := $(LIBPATH)/MovingObjectsLibrary.jar
JARS := $(JARS):$(LIBPATH)/jxmapviewer2-2.3.jar
JARS := $(JARS):$(LIBPATH)/commons-logging-1.1.1.jar

CLASSPATH := $(CLASSPATH):$(JARS):.
JAVAFILES =$(shell find . -name "*.java")

default:jar

.PHONY:all
all:makedir files

files: .javac-stamp

# compile all sources with a single javac invocation instead of one per file
.javac-stamp: $(JAVAFILES)
	$(JAVAC) -classpath $(CLASSPATH) -d bin $?
	@touch $@


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
	rm -f jar/MapViewer.jar .javac-stamp
	rm -frd bin/*
