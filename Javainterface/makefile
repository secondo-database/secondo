# Filename: SecondoJava/makefile
#	
# a change: Oct. 2002, M. Spiekermann
# last change: 05-2003, Th. Behr

include ./makejava.inc

# Some variables classifying the JavaInterface files.

# library files
JILIB_RUNTIME := ./lib/$(notdir $(CUPCLASSES_RUNTIME)) 
JILIB_ALL :=  $(shell find ./lib -name "*.zip") $(SJLIB_RUNTIME)

	
#########   RULES  SECTION   ####################


.PHONY: all # default rule prints this message
all:  ji

.PHONY: ji # make all classes
ji: $(JISCRIPT_FILES) 
	(cd sj; make all)


####### CLEAN UP RULES ############


.PHONY: clean # delete nearly all generated files
clean: 
	rm -f *.list *.gz *.tar *.jar *.bat
	(cd sj; make clean)

.PHONY: mrproper # delete really all generated files
mrproper:
	(cd sj; make mrproper)


