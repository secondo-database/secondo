#This file is part of SECONDO.

#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

########################################################################
#
# SECONDO makefile for algebra manager and algebra modules
#
########################################################################

include ../makefile.env

ALL_ALGEBRA_DIRS := $(shell find $(BUILDDIR)/Algebras -maxdepth 1 -type d ! -name "Algebras" ! -name "Management" ! -name "CVS" -printf "%f ")

ifdef SECONDO_ACTIVATE_ALL_ALGEBRAS
$(warning Activating all Algebras!)
USED_ALGEBRA_DIRS := $(ALL_ALGEBRA_DIRS)
else
USED_ALGEBRA_DIRS := $(ALGEBRA_DIRS)
endif


.PHONY: all
all: examples buildlibs

../makefile.algebras: ../makefile.algebras.sample
	$(cp-config-file)

SPECFILES := $(wildcard $(addsuffix /*.spec, $(USED_ALGEBRA_DIRS) ))
EXAMPLEFILES := $(wildcard $(addsuffix /*.examples, $(USED_ALGEBRA_DIRS) ))

.PHONY: examples
examples:
	@if [ ! -d ../bin/tmp ]; then mkdir -p ../bin/tmp; fi
	cp $(EXAMPLEFILES) ../bin/tmp

specs: specs.test
	@chmod u+x *.awk
	@rm -f specs
	@for file in $(SPECFILES); do \
          if !((echo -e "\n# file $$file \n" && cat $$file) >> specs) then \
            exit 1; \
          fi \
        done
	@echo -e "\n *** Master spec file specs succesfully concatenated. ***\n"


PARSERDIR := ../Tools/Parser/SpecParser
specs.test: $(SPECFILES) ../makefile.algebras
	@echo -e "\n *** Testing correctness of spec files ***\n";
	@for file in $(SPECFILES); do \
          echo -e "checking $$file ... "; \
          if !($(PARSERDIR)/Parser < $$file &> /dev/null) then \
            $(PARSERDIR)/Parser2 < $$file; \
            exit 1; \
          fi \
        done
	@touch $@
	@rm -f tokens lexrules yaccrules1 yaccrules2


.PHONY:buildlibs
buildlibs: examples
	@echo -e "\n *** Building algebra libs ***\n";
	for dir in $(USED_ALGEBRA_DIRS); do \
          if [ -e $$dir/makefile ]; then \
            cd $$dir; if !($(MAKE)) then \
	      exit 1; \
	    fi; cd ..; \
          fi \
	done
	@echo -e "\n *** Algebra libs successfully created ***\n";


.PHONY: clean
clean:
	rm -f specs specs.test
	for dir in $(USED_ALGEBRA_DIRS); do \
          cd $$dir; if !($(MAKE) clean) then \
            exit 1; \
          fi; cd ..; \
        done

.PHONY: realclean
realclean:
	rm -f specs specs.test
	for dir in $(ALL_ALGEBRA_DIRS); do \
          cd $$dir; if !($(MAKE) clean) then \
            exit 1; \
          fi; cd ..; \
        done
