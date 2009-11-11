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
DATABASE_DIR1=$(HOME)/secondo-databases/intern
DATABASE_DIR2=$(HOME)/secondo-databases/extern

# test location
TESTHOME=$(SECONDO_BUILD_DIR)/Algebras/ExtRelation-2/experiments

# directory where the log files will be stored
LOGDIR=$(TESTHOME)/logs

# name of the test summary file
SUMMARY=$(TESTHOME)/logs/testsort_summary.$(shell date +%d%m%y_%k%M%S).log

# file extension for experiment
TESTEXT=test

# build directory of the random text relation generator
TEXTGEN_DIR = $(SECONDO_BUILD_DIR)/Tools/Generators/TextRelations

# random text relation directory
TEXTGEN = $(TEXTGEN_DIR)/createRandomTextRelation

# variables which controls relation size (for testing only)
100MB=104857600
50MB=52428800
10MB=10485760
1MB=1048576

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
	@make --directory=$(TEXTGEN_DIR)

# --------------------------------------------
# 1 MB Relations
# --------------------------------------------

# creates a 1 MB relation with tuple size 64 bytes
.PHONY: R1MB_T64B
R1MB_T64B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 64 $@ > $@

# creates a 1 MB relation with tuple size 128 bytes
.PHONY: R1MB_T128B
R1MB_T128B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 128 $@ > $@

# creates a 1 MB relation with tuple size 256 bytes
.PHONY: R1MB_T256B
R1MB_T256B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 256 $@ > $@

# creates a 1 MB relation with tuple size 512 bytes
.PHONY: R1MB_T512B
R1MB_T512B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 512 $@ > $@

# creates a 1 MB relation with tuple size 1 Kbytes
.PHONY: R1MB_T1KB
R1MB_T1KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 1024 $@ > $@

# creates a 1 MB relation with tuple size 2 Kbytes
.PHONY: R1MB_T2KB
R1MB_T2KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 2048 $@ > $@

# creates a 1 MB relation with tuple size 4 Kbytes
.PHONY: R1MB_T4KB
R1MB_T4KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 4096 $@ > $@

# creates a 1 MB relation with tuple size 8 Kbytes
.PHONY: R1MB_T8KB
R1MB_T8KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 8192 $@ > $@

# creates a 1 MB relation with tuple size 16 Kbytes
.PHONY: R1MB_T16KB
R1MB_T16KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 16384 $@ > $@

# creates a 1 MB relation with tuple size 32 Kbytes
.PHONY: R1MB_T32KB
R1MB_T32KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 32768 $@ > $@

# creates a 1 MB relation with tuple size 64 Kbytes
.PHONY: R1MB_T64KB
R1MB_T64KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(1MB) 65536 $@ > $@

# --------------------------------------------
# 10 MB Relations
# --------------------------------------------

# creates a 10 MB relation with tuple size 64 bytes
.PHONY: R10MB_T64B
R10MB_T64B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 64 $@ > $@

# creates a 10 MB relation with tuple size 128 bytes
.PHONY: R10MB_T128B
R10MB_T128B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 128 $@ > $@

# creates a 10 MB relation with tuple size 256 bytes
.PHONY: R10MB_T256B
R10MB_T256B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 256 $@ > $@

# creates a 10 MB relation with tuple size 512 bytes
.PHONY: R10MB_T512B
R10MB_T512B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 512 $@ > $@

# creates a 10 MB relation with tuple size 1 Kbytes
.PHONY: R10MB_T1KB
R10MB_T1KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 1024 $@ > $@

# creates a 10 MB relation with tuple size 2 Kbytes
.PHONY: R10MB_T2KB
R10MB_T2KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 2048 $@ > $@

# creates a 10 MB relation with tuple size 4 Kbytes
.PHONY: R10MB_T4KB
R10MB_T4KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 4096 $@ > $@

# creates a 10 MB relation with tuple size 8 Kbytes
.PHONY: R10MB_T8KB
R10MB_T8KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 8192 $@ > $@

# creates a 10 MB relation with tuple size 16 Kbytes
.PHONY: R10MB_T16KB
R10MB_T16KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 16384 $@ > $@

# creates a 10 MB relation with tuple size 32 Kbytes
.PHONY: R10MB_T32KB
R10MB_T32KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 32768 $@ > $@

# creates a 10 MB relation with tuple size 64 Kbytes
.PHONY: R10MB_T64KB
R10MB_T64KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(10MB) 65536 $@ > $@

# --------------------------------------------
# 50 MB Relations
# --------------------------------------------

# creates a 50 MB relation with tuple size 64 bytes
.PHONY: R50MB_T64B
R50MB_T64B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 64 $@ > $@

# creates a 50 MB relation with tuple size 128 bytes
.PHONY: R50MB_T128B
R50MB_T128B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 128 $@ > $@

# creates a 50 MB relation with tuple size 256 bytes
.PHONY: R50MB_T256B
R50MB_T256B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 256 $@ > $@

# creates a 50 MB relation with tuple size 512 bytes
.PHONY: R50MB_T512B
R50MB_T512B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 512 $@ > $@

# creates a 50 MB relation with tuple size 1 Kbytes
.PHONY: R50MB_T1KB
R50MB_T1KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 1024 $@ > $@

# creates a 50 MB relation with tuple size 2 Kbytes
.PHONY: R50MB_T2KB
R50MB_T2KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 2048 $@ > $@

# creates a 50 MB relation with tuple size 4 Kbytes
.PHONY: R50MB_T4KB
R50MB_T4KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 4096 $@ > $@

# creates a 50 MB relation with tuple size 8 Kbytes
.PHONY: R50MB_T8KB
R50MB_T8KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 8192 $@ > $@

# creates a 50 MB relation with tuple size 16 Kbytes
.PHONY: R50MB_T16KB
R50MB_T16KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 16384 $@ > $@

# creates a 50 MB relation with tuple size 32 Kbytes
.PHONY: R50MB_T32KB
R50MB_T32KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 32768 $@ > $@

# creates a 50 MB relation with tuple size 64 Kbytes
.PHONY: R50MB_T64KB
R50MB_T64KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(50MB) 65536 $@ > $@

# --------------------------------------------
# 100 MB Relations
# --------------------------------------------

# creates a 100 MB relation with tuple size 64 bytes
.PHONY: R100MB_T64B
R100MB_T64B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 64 $@ > $@

# creates a 100 MB relation with tuple size 128 bytes
.PHONY: R100MB_T128B
R100MB_T128B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 128 $@ > $@

# creates a 100 MB relation with tuple size 256 bytes
.PHONY: R100MB_T256B
R100MB_T256B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 256 $@ > $@

# creates a 100 MB relation with tuple size 512 bytes
.PHONY: R100MB_T512B
R100MB_T512B: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 512 $@ > $@

# creates a 100 MB relation with tuple size 1 Kbytes
.PHONY: R100MB_T1KB
R100MB_T1KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 1024 $@ > $@

# creates a 100 MB relation with tuple size 2 Kbytes
.PHONY: R100MB_T2KB
R100MB_T2KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 2048 $@ > $@

# creates a 100 MB relation with tuple size 4 Kbytes
.PHONY: R100MB_T4KB
R100MB_T4KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 4096 $@ > $@

# creates a 100 MB relation with tuple size 8 Kbytes
.PHONY: R100MB_T8KB
R100MB_T8KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 8192 $@ > $@

# creates a 100 MB relation with tuple size 16 Kbytes
.PHONY: R100MB_T16KB
R100MB_T16KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 16384 $@ > $@

# creates a 100 MB relation with tuple size 32 Kbytes
.PHONY: R100MB_T32KB
R100MB_T32KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 32768 $@ > $@

# creates a 100 MB relation with tuple size 64 Kbytes
.PHONY: R100MB_T64KB
R100MB_T64KB: textgen
	@echo ""
	@echo "Creating relation.." $@
	@rm -f $@ 
	@$(TEXTGEN) $(100MB) 65536 $@ > $@

# --------------------------------------------
# Cleanup of database directory
# --------------------------------------------

.PHONY: clean_db
clean_db:
	@echo ""
	@echo "Cleaning database directory.."
	@echo $(DATABASE_DIR1)
	@rm -f -r $(DATABASE_DIR1)/*
	@echo $(DATABASE_DIR2)
	@rm -f -r $(DATABASE_DIR2)/*

# --------------------------------------------
# Experiments
# --------------------------------------------

tuplebuffer: clean_db R100MB_T64B R100MB_T256B R100MB_T1KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	$(call doTest, $@)

sort_iobuffer: clean_db R100MB_T512B R100MB_T8KB R100MB_T64KB 
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	$(call doTest, $@)

sort_max_fan_in: clean_db R100MB_T1KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	$(call doTest, $@)

sort_huge_data: clean_db R100MB_T1KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMSecondoHome=/home/sven/secondo-databases/extern \
	$(call doTest, sort_5G)
	$(call doTest, sort_10G)
	$(call doTest, sort_15G)
	$(call doTest, sort_20G)

sort_progress_estimation: clean_db R10MB_T64B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, sort_progress_estimation1)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/sort_progress_estimation1_proglogt.csv
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, sort_progress_estimation2)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/sort_progress_estimation2_proglogt.csv
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, sort_progress_estimation3)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/sort_progress_estimation3_proglogt.csv
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv

sort_benchmark_64B: clean_db R1MB_T64B R10MB_T64B R50MB_T64B R100MB_T64B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)

sort_benchmark_128B: clean_db R1MB_T128B R10MB_T128B R50MB_T128B R100MB_T128B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)

sort_benchmark_256B: clean_db R1MB_T256B R10MB_T256B R50MB_T256B R100MB_T256B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)

sort_benchmark_512B: clean_db R1MB_T512B R10MB_T512B R50MB_T512B R100MB_T512B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)

sort_benchmark_1KB: clean_db R1MB_T1KB R10MB_T1KB R50MB_T1KB R100MB_T1KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)

sort_benchmark_2KB: clean_db R1MB_T2KB R10MB_T2KB R50MB_T2KB R100MB_T2KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)

sort_benchmark_4KB: clean_db R1MB_T4KB R10MB_T4KB R50MB_T4KB R100MB_T4KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@)

sort_benchmark_8KB: clean_db R1MB_T8KB R10MB_T8KB R50MB_T8KB R100MB_T8KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@)

sort_benchmark_16KB: clean_db R1MB_T16KB R10MB_T16KB R50MB_T16KB R100MB_T16KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@)

sort_benchmark_32KB: clean_db R1MB_T32KB R10MB_T32KB R50MB_T32KB R100MB_T32KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@)

sort_benchmark_64KB: clean_db R1MB_T64KB R10MB_T64KB R50MB_T64KB R100MB_T64KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@)

test: clean_db 
	@echo ""
	@echo "Running testset"
	$(call doTest, sort_max_fan_in)
	$(call doTest, sort_max_fan_in)

hashjoin_number_of_partitions: clean_db
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)

dummy: clean_db #R1G_T512B R1G_T8KB R1G_T64KB 
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	$(call doTest, dummy)




