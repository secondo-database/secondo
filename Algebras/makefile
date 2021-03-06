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


ifdef SECONDO_ACTIVATE_ALL_ALGEBRAS
$(warning Activating all Algebras!)
USED_ALGEBRA_DIRS := $(ALL_ALGEBRA_DIRS)
else
USED_ALGEBRA_DIRS := $(ALGEBRA_DIRS)
endif


.PHONY: all
all: buildlibs

../makefile.algebras: ../makefile.algebras.sample
	$(cp-config-file)

SPECFILES := $(wildcard $(addsuffix /*.spec, $(ALL_ALGEBRA_DIRS) ))
EXAMPLEFILES := $(wildcard $(addsuffix /*.examples, $(USED_ALGEBRA_DIRS) ))

.PHONY: examples
examples:
	@for dir in "../bin/tmp" "../Optimizer/tmp"; do \
	  mkdir -p $$dir; \
	  echo "Removing old .examples in $$dir"; \
	  rm -rf $$dir/*.examples; \
          echo "Copying .examples files to $$dir"; \
	  cp $(EXAMPLEFILES) $$dir; \
	done 

specs: specs.test
	@chmod u+x *.awk
	@rm -f specs
	@for file in $(SPECFILES); do \
          if !((echo && echo "# file $$file " && echo && cat $$file) >> specs) then \
            exit 1; \
          fi \
        done
	@echo && echo  " *** Master spec file specs succesfully concatenated. ***" && echo


PARSERDIR := ../Tools/Parser/SpecParser
specs.test: $(SPECFILES) ../makefile.algebras
	@echo && echo " *** Testing correctness of spec files ***" && echo;
	@for file in $(SPECFILES); do \
          echo  "checking $$file ... "; \
          if !($(PARSERDIR)/Parser < $$file &> /dev/null) then \
            $(PARSERDIR)/Parser2 < $$file; \
            exit 1; \
          fi \
        done
	@touch $@
	@rm -f tokens lexrules yaccrules1 yaccrules2


.PHONY:buildlibs
buildlibs: examples
	@echo && echo  " *** Building algebra libs ***" && echo;
	for dir in $(USED_ALGEBRA_DIRS); do \
          if [ -e $$dir/makefile ]; then \
            if !($(MAKE) -C $$dir) then \
	          exit 1; \
	        fi; \
          fi \
	done
	@echo && echo " *** Algebra libs successfully created ***" && echo;


.PHONY: clean
clean:
	rm -f specs specs.test
	for dir in $(USED_ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir clean) then \
            exit 1; \
          fi; \
        done

.PHONY: realclean
realclean:
	rm -f specs specs.test
	for dir in $(ALL_ALGEBRA_DIRS); do \
	  if [ -e $$dir/makefile ]; then \
            if !($(MAKE) -C $$dir clean) then \
              exit 1; \
            fi; \
	  fi; \
        done
