########################################################################
#
# SECONDO makefile for algebra manager and algebra modules
#
########################################################################

include ../makefile.env

ALL_ALGEBRA_DIRS := $(ALGEBRA_DIRS)
ifeq ($(USE_JNI),"true")
ALL_ALGEBRA_DIRS += $(JNI_ALGEBRA_DIRS)
endif


.PHONY: all
all: buildlibs

LS_COMMAND = $(addsuffix /*.spec, $(ALL_ALGEBRA_DIRS) )
#SPECFILES = $(shell ls $(LS_COMMAND))


specs: testspecs $(SPECFILES) ../makefile.algebras
	@chmod u+x *.awk
	@rm -f specs
	@for file in */*.spec; do \
          if !((cat $$file && echo -e "\n# file $$file \n") >> specs) then \
            exit 1; \
          fi \
        done
	cat $@ | sort -b | ./specs.awk
	@echo -e "\n *** Master spec file specs succesfully concatenated. ***\n"


PARSERDIR := ../Tools/Parser/SpecParser
.PHONY: testspecs
testspecs:
	@echo -e "\n *** Testing correctness of spec files ***\n";
	@for file in */*.spec; do \
          echo -e "checking $$file ... "; \
          if !($(PARSERDIR)/Parser < $$file &> /dev/null) then \
            $(PARSERDIR)/Parser2 < $$file \
            exit 1; \
          fi \
        done
	@rm -f tokens yaccrules1 yaccrules2


.PHONY:buildlibs
buildlibs:
	@echo -e "\n *** Building algebra libs ***\n";
	for dir in $(ALL_ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done
	@echo -e "\n *** Algebra libs successfully created ***\n";


.PHONY: clean
clean:
	rm -f specs specs.tmp
	for dir in $(ALL_ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir clean) then \
            exit 1; \
          fi \
        done
