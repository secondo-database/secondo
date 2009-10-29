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



# --------------------------------------------
# Configuration
# --------------------------------------------

# database directory
DATABASE_DIR=$(HOME)/secondo-databases/intern

# test location
TESTHOME=$(SECONDO_BUILD_DIR)/Algebras/ExtRelation-2/experiments

# directory where the log files will be stored
LOGDIR=$(TESTHOME)/logs

# name of the test summary file
SUMMARY=$(TESTHOME)/logs/testsort_summary.$(shell date +%d%m%y_%k%M%S).log

# file extension for tests
TESTEXT=test

# build directory of the random text relation generator
TEXTGEN_DIR = $(SECONDO_BUILD_DIR)/Tools/Generators/TextRelations

# random text relation directory
TEXTGEN = $(TEXTGEN_DIR)/createRandomTextRelation


# --------------------------------------------
# Functions
# --------------------------------------------

# $(call doTest, name)
define doTest
  START=`date`; \
  TEST=$(join $(TESTHOME)/,$1).$(TESTEXT); \
  TESTLOG=$(join $(LOGDIR)/,$1).$(shell date +%d%m%y_%k%M%S).log; \
  if [[ ! -d "$(LOGDIR)" ]]; then \
      mkdir $(LOGDIR); \
  fi; \
  cd $(SECONDO_BUILD_DIR)/bin; \
  TestRunner --no-tmp -i $$TEST | tee $$TESTLOG;\
  echo "" | tee -a $(SUMMARY); \
  echo "----------------------------------------" | tee -a $(SUMMARY); \
  echo $$TEST | tee -a $(SUMMARY); \
  echo "----------------------------------------" | tee -a $(SUMMARY); \
  echo | tee -a $(SUMMARY); \
  grep "TEST SUMMARY :" -A 2 $$TESTLOG | tee -a $(SUMMARY); \
  echo | tee -a $(SUMMARY); \
  echo "Start: " $$START | tee -a $(SUMMARY); \
  echo "End: " `date` | tee -a $(SUMMARY)
endef

# --------------------------------------------
# Targets
# --------------------------------------------

all:
	@echo ""
	@echo ""
	@echo " sort_iobuffer: "
	@echo ""

# build the random text relation generator
textgen: $(TEXTGEN)
	@echo ""
	@echo "Building generator.." 
	$(TEXTGEN)
	@make --directory=$(TEXTGEN_DIR)

# creates a 1 GB relation with tuple size 64 bytes
.PHONY: R1G_T64B
R1G_T64B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 64 $@ > $@

# creates a 1 GB relation with tuple size 128 bytes
.PHONY: R1G_T128B
R1G_T128B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 128 $@ > $@

# creates a 1 GB relation with tuple size 256 bytes
.PHONY: R1G_T256B
R1G_T256B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 256 $@ > $@

# creates a 1 GB relation with tuple size 512 bytes
.PHONY: R1G_T512B
R1G_T512B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 512 $@ > $@

# creates a 1 GB relation with tuple size 1 Kbytes
.PHONY: R1G_T1KB
R1G_T1KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 1024 $@ > $@

# creates a 1 GB relation with tuple size 2 Kbytes
.PHONY: R1G_T2KB
R1G_T2KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 2048 $@ > $@

# creates a 1 GB relation with tuple size 4 Kbytes
.PHONY: R1G_T4KB
R1G_T4KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 4096 $@ > $@

# creates a 1 GB relation with tuple size 8 Kbytes
.PHONY: R1G_T8KB
R1G_T8KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 8192 $@ > $@

# creates a 1 GB relation with tuple size 16 Kbytes
.PHONY: R1G_T16KB
R1G_T16KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 16384 $@ > $@

# creates a 1 GB relation with tuple size 32 Kbytes
.PHONY: R1G_T32KB
R1G_T32KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 32768 $@ > $@

# creates a 1 GB relation with tuple size 64 Kbytes
.PHONY: R1G_T64KB
R1G_T64KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) 1073741824 65536 $@ > $@

.PHONY: clean_db
clean_db:
	@echo ""
	@echo "Cleaning database directory.."
	@echo $(DATABASE_DIR)
	@rm -f -r $(DATABASE_DIR)/*

sort_iobuffer: clean_db #R1G_T512B R1G_T8KB R1G_T64KB 
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	$(call doTest, $@)

dummy: clean_db #R1G_T512B R1G_T8KB R1G_T64KB 
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	$(call doTest, dummy)




