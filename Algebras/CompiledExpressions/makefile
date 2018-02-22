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
# generic makefile for an algebra module 
#
########################################################################

#shared := yes

# inlcude global definitions
include ../../makefile.env

CURRENT_ALGEBRA := CompiledExpressionsAlgebra
#ALGEBRA_DEPENDENCIES := StandardAlgebra
#ALGEBRA_DEPENDENCIES += RelationAlgebra

CE_EXTERN_MAKE_DIR := ../Management
CE_EXTERN_MAKE_FILE := AlgebraManager.cpp
TMP_CURRENT_DIR := $(PWD)

# expand compile flags if necessary
#CCFLAGS += -Werror

# create objects and algebra library. The values for the variables
# OBJECTS and LIBOBJ are computed automatically by makefile.env 
.PHONY: all
all: check_dependencies $(OBJECTS) $(LIBOBJ)

.PHONY: prepare_kernel_header
prepare_kernel_header:
	@echo && echo " *** Include or Remove CE-Algebra in Secondo *** " && echo;
ifneq (,$(findstring $(CURRENT_ALGEBRA),$(ALGEBRAS)))
	if [ "`cat LinkCEAlgebraFunction.h | grep '^\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_' | wc -l`" -eq "1" ] ; then \
		sed -i "s/^\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_/#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_/g" LinkCEAlgebraFunction.h ;\
	fi
else
	if [ "`cat LinkCEAlgebraFunction.h | grep '^#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_' | wc -l`" -eq "1" ] ; then \
		sed -i "s/^#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_/\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_/g" LinkCEAlgebraFunction.h ;\
	fi
endif
ifeq ($(shared),yes)
	if [ "`cat LinkCEAlgebraFunction.h | grep '^#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_' | wc -l`" -eq "1" ] ; then \
		sed -i "s/^#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_/\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_/g" LinkCEAlgebraFunction.h ;\
	fi
else
	if [ "`cat LinkCEAlgebraFunction.h | grep '^\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_' | wc -l`" -eq "1" ] ; then \
		sed -i "s/^\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_/#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_/g" LinkCEAlgebraFunction.h ;\
	fi
endif
	for dir in $(CE_EXTERN_MAKE_DIR); do \
		if [ -e $$dir/makefile ]; then \
			cd $$dir; \
			for file in $(CE_EXTERN_MAKE_FILE); do \
				if [ -e $$dir/$$file ]; then \
				touch $$dir/$$file; \
				fi; \
			done; \
			if !($(MAKE)) then \
				exit 1; \
			fi; \
			cd $$TMP_CURRENT_DIR; \
		fi \
	done
	@echo && echo " *** Include or Remove CE-Algebra in Secondo finish ***" && echo;

.PHONY: prepare_ext_header
prepare_ext_header:
	@echo && echo " *** Prepare CompiledExpressionAlgebra-Headerfile for use in Secondo  *** " && echo;
ifneq (,$(findstring $(CURRENT_ALGEBRA),$(ALGEBRAS)))
	if [ "`cat LinkCEAlgebraFunction.h | grep '^\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_' | wc -l`" -eq "1" ] ; then \
		sed -i "s/^\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_/#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_/g" LinkCEAlgebraFunction.h ;\
	fi
else
	if [ "`cat LinkCEAlgebraFunction.h | grep '^#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_' | wc -l`" -eq "1" ] ; then \
		sed -i "s/^#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_/\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_/g" LinkCEAlgebraFunction.h ;\
	fi
endif
ifeq ($(shared),yes)
	@echo && echo " *** Shared=YES  *** |$(shared)| " && echo;
	if [ "`cat LinkCEAlgebraFunction.h | grep '^#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_' | wc -l`" -eq "1" ] ; then \
		sed -i "s/^#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_/\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_/g" LinkCEAlgebraFunction.h ;\
	fi
else
	@echo && echo " *** Shared=NO  *** |$(shared)| " && echo;
	if [ "`cat LinkCEAlgebraFunction.h | grep '^\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_' | wc -l`" -eq "1" ] ; then \
		sed -i "s/^\/\/#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_/#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_/g" LinkCEAlgebraFunction.h ;\
	fi
endif
	@echo && echo " *** -Finish- Prepare CompiledExpressionAlgebra-Headerfile for use in Secondo -Finish- *** " && echo;


check_dependencies:
	$(check-algebra-dependencies)


# include pre-computed dependencies
ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif


# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)



# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)
	rm -f *.toc

