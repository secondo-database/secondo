include ../makefile.env

.PHONY: all
all:
	make -C jsrc all
	make -C lib all
	make -C src all

.PHONY:doc
doc:
	javadoc -classpath $(JPL_CLASS_TARGET) -sourcepath $(JPL_JAVA_SOURCES) -d $(JPL_DOC_TARGET) jpl jpl.fli

.PHONY: clean
clean:
	make -C lib clean
	make -C src clean
	rm -rf $$(find doc ! -path "*CVS*" ! -name "doc")
