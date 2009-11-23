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

# random text relation directory
TEXTGENSEL = $(TEXTGEN_DIR)/createTextRelationsWithSel

# variables which controls relation size (for testing only)
100MB=104857600
50MB=52428800
10MB=10485760
1MB=1048576

TUPLESIZE_256=256
TUPLES_256_50MB=204800
TUPLES_256_25MB=102400
TUPLES_256_5MB=20480


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
# Relations with specific Selectivity
# --------------------------------------------

# Selectivity 0,001%
.PHONY: RELPAIR_SEL0_001
RELPAIR_SEL0_001:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@A
	$(TEXTGENSEL) 64 $(TUPLES_256_50MB) $(TUPLES_256_5MB) 0.00001 $@A_64 $@B_64
	$(TEXTGENSEL) 128 $(TUPLES_256_50MB) $(TUPLES_256_5MB) 0.00001 $@A_128 $@B_128
	$(TEXTGENSEL) 256 $(TUPLES_256_50MB) $(TUPLES_256_5MB) 0.00001 $@A_256 $@B_256
	$(TEXTGENSEL) 512 $(TUPLES_256_50MB) $(TUPLES_256_5MB) 0.00001 $@A_512 $@B_512
	$(TEXTGENSEL) 1024 $(TUPLES_256_50MB) $(TUPLES_256_5MB) 0.00001 $@A_1024 $@B_1024

# Selectivity 1%
.PHONY: RELPAIR_SEL1
RELPAIR_SEL1:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.01 $@A $@B

# Selectivity 5%
.PHONY: RELPAIR_SEL5
RELPAIR_SEL5:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.05 $@A $@B

# Selectivity 10%
.PHONY: RELPAIR_SEL10
RELPAIR_SEL10:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.1 $@A $@B

# Selectivity 20%
.PHONY: RELPAIR_SEL20
RELPAIR_SEL20:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.2 $@A $@B

# Selectivity 30%
.PHONY: RELPAIR_SEL30
RELPAIR_SEL30:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.3 $@A $@B

# Selectivity 40%
.PHONY: RELPAIR_SEL40
RELPAIR_SEL40:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.4 $@A $@B

# Selectivity 50%
.PHONY: RELPAIR_SEL50
RELPAIR_SEL50:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.5 $@A $@B

# Selectivity 60%
.PHONY: RELPAIR_SEL60
RELPAIR_SEL60:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.6 $@A $@B

# Selectivity 70%
.PHONY: RELPAIR_SEL70
RELPAIR_SEL70:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.7 $@A $@B

# Selectivity 80%
.PHONY: RELPAIR_SEL80
RELPAIR_SEL80:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.8 $@A $@B

# Selectivity 90%
.PHONY: RELPAIR_SEL90
RELPAIR_SEL90:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 0.9 $@A $@B

# Selectivity 100%
.PHONY: RELPAIR_SEL100
RELPAIR_SEL100:
	@echo ""
	@echo "Creating relation pair.." $@
	@rm -f $@A $@B
	@$(TEXTGENSEL) 256 20480 4096 1 $@A $@B

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

sort_max_fan_in: clean_db R100MB_T1KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	$(call doTest, $@)

sort_huge_data: clean_db R100MB_T1KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMSecondoHome=/home/sven/secondo-databases/extern \
	$(call doTest, sort_1G)
	$(call doTest, sort_5G)
	$(call doTest, sort_10G)
	$(call doTest, sort_15G)
	$(call doTest, sort_20G)

sort_progress_estimation: clean_db R10MB_T64B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, $@1)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/$@1_proglogt.csv
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, $@2)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/$@2_proglogt.csv
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, $@3)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/$@3_proglogt.csv
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv


.PHONY: sort_benchmark_64B
sort_benchmark_64B: clean_db R1MB_T64B R10MB_T64B R50MB_T64B R100MB_T64B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

.PHONY: sort_benchmark_128B
sort_benchmark_128B: clean_db R1MB_T128B R10MB_T128B R50MB_T128B R100MB_T128B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

.PHONY: sort_benchmark_256B
sort_benchmark_256B: clean_db R1MB_T256B R10MB_T256B R50MB_T256B R100MB_T256B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

.PHONY: sort_benchmark_512B
sort_benchmark_512B: clean_db R1MB_T512B R10MB_T512B R50MB_T512B R100MB_T512B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

.PHONY: sort_benchmark_1KB
sort_benchmark_1KB: clean_db R1MB_T1KB R10MB_T1KB R50MB_T1KB R100MB_T1KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

sort_benchmark_2KB: clean_db R1MB_T2KB R10MB_T2KB R50MB_T2KB R100MB_T2KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

sort_benchmark_4KB: clean_db R1MB_T4KB R10MB_T4KB R50MB_T4KB R100MB_T4KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

sort_benchmark_8KB: clean_db R1MB_T8KB R10MB_T8KB R50MB_T8KB R100MB_T8KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

sort_benchmark_16KB: clean_db R1MB_T16KB R10MB_T16KB R50MB_T16KB R100MB_T16KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

sort_benchmark_32KB: clean_db R1MB_T32KB R10MB_T32KB R50MB_T32KB R100MB_T32KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

sort_benchmark_64KB: clean_db R1MB_T64KB R10MB_T64KB R50MB_T64KB R100MB_T64KB
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@1)
	$(call doTest, $@2)
	$(call doTest, $@3)
	$(call doTest, $@4)
	$(call doTest, $@5)
	$(call doTest, $@6)

sort_benchmark: clean_db \
R1MB_T64B R1MB_T128B R1MB_T256B R1MB_T512B R1MB_T1KB R1MB_T2KB R1MB_T4KB R1MB_T8KB R1MB_T16KB  R1MB_T32KB R1MB_T64KB \
R10MB_T64B R10MB_T128B R10MB_T256B R10MB_T512B R10MB_T1KB R10MB_T2KB R10MB_T4KB R10MB_T8KB R10MB_T16KB  R10MB_T32KB R10MB_T64KB \
R50MB_T64B R50MB_T128B R50MB_T256B R50MB_T512B R50MB_T1KB R50MB_T2KB R50MB_T4KB R50MB_T8KB R50MB_T16KB  R50MB_T32KB R50MB_T64KB \
R100MB_T64B R100MB_T128B R100MB_T256B R100MB_T512B R100MB_T1KB R100MB_T2KB R100MB_T4KB R100MB_T8KB R100MB_T16KB  R100MB_T32KB R100MB_T64KB 
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_64B1)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_64B2)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_64B3)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_64B4)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_64B5)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_64B6)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_128B1)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_128B2)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_128B3)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_128B4)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_128B5)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_128B6)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_256B1)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_256B2)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_256B3)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_256B4)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_256B5)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_256B6)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_512B1)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_512B2)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_512B3)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_512B4)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_512B5)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_512B6)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_1KB1)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_1KB2)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_1KB3)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_1KB4)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_1KB5)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@_1KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_2KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_2KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_2KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_2KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_2KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_2KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_4KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_4KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_4KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_4KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_4KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_4KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_8KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_8KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_8KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_8KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_8KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_8KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_16KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_16KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_16KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_16KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_16KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_16KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_32KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_32KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_32KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_32KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_32KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_32KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_64KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_64KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_64KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_64KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_64KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@_64KB6)

hashjoin_number_of_partitions: clean_db
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	rm -f REL256_A25x* REL256_B25x*
	$(TEXTGENSEL) 256 102400 8192 0.001 REL256_A25x2 REL256_B25x2
	$(TEXTGENSEL) 256 102400 20480 0.001 REL256_A25x5 REL256_B25x5
	$(TEXTGENSEL) 256 102400 40960 0.001 REL256_A25x10 REL256_B25x10
	$(TEXTGENSEL) 256 102400 61440 0.001 REL256_A25x15 REL256_B25x15
	$(TEXTGENSEL) 256 102400 81920 0.001 REL256_A25x20 REL256_B25x20
	$(TEXTGENSEL) 256 102400 102400 0.001 REL256_A25x25 REL256_B25x25
	$(TEXTGENSEL) 256 102400 122880 0.001 REL256_A25x30 REL256_B25x30
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)

hashjoin_switch_intern_extern: clean_db R50MB_T256B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@)

hashjoin_subpartitioning: clean_db
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	rm -f $@_REL256A $@_REL256B
	$(TEXTGENSEL) 256 102400 102400 0.00001 $@_REL256A $@_REL256B 
	export SECONDO_PARAMMaxMemPerOperator=16384; \
	$(call doTest, $@)
	rm -f $@_REL256A $@_REL256B

hashjoin_hybrid_vs_grace: clean_db
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	rm -f -v $@_A* $@_B*
	$(TEXTGENSEL) 256 40960 40960 0.00001 $@_A10x10 $@_B10x10
	$(TEXTGENSEL) 256 81920 40960 0.00001 $@_A20x10 $@_B20x10
	$(TEXTGENSEL) 256 122880 40960 0.00001 $@_A30x10 $@_B30x10
	$(TEXTGENSEL) 256 163840 40960 0.00001 $@_A40x10 $@_B40x10
	$(TEXTGENSEL) 256 204800 40960 0.00001 $@_A50x10 $@_B50x10
	$(TEXTGENSEL) 256 245760 40960 0.00001 $@_A60x10 $@_B60x10
	$(TEXTGENSEL) 256 286720 40960 0.00001 $@_A70x10 $@_B70x10
	$(TEXTGENSEL) 256 327680 40960 0.00001 $@_A80x10 $@_B80x10
	$(TEXTGENSEL) 256 368640 40960 0.00001 $@_A90x10 $@_B90x10
	$(TEXTGENSEL) 256 409600 40960 0.00001 $@_A100x10 $@_B100x10
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)
	rm -f -v $@_A* $@_B*

hashjoin_asymmetric_queries: clean_db R50MB_T64B R50MB_T256B R50MB_T512B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=1024; \
	$(call doTest, $@)

hashjoin_selectivity: clean_db RELPAIR_SEL1 RELPAIR_SEL5 RELPAIR_SEL10 RELPAIR_SEL20 \
RELPAIR_SEL30 RELPAIR_SEL40 RELPAIR_SEL50 RELPAIR_SEL60 RELPAIR_SEL70 RELPAIR_SEL80 \
RELPAIR_SEL90 RELPAIR_SEL100 
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, $@)

hashjoin_benchmark: clean_db RELPAIR_SEL0_001
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=64; \
	$(call doTest, $@_1)
#	export SECONDO_PARAMMaxMemPerOperator=128; \
#	$(call doTest, $@2)
#	export SECONDO_PARAMMaxMemPerOperator=256; \
#	$(call doTest, $@3)
#	export SECONDO_PARAMMaxMemPerOperator=512; \
#	$(call doTest, $@4)
#	export SECONDO_PARAMMaxMemPerOperator=768; \
#	$(call doTest, $@5)
#	export SECONDO_PARAMMaxMemPerOperator=1024; \
#	$(call doTest, $@6)
#	export SECONDO_PARAMMaxMemPerOperator=1280; \
#	$(call doTest, $@7)
#	export SECONDO_PARAMMaxMemPerOperator=1536; \
#	$(call doTest, $@8)
#	export SECONDO_PARAMMaxMemPerOperator=1792; \
#	$(call doTest, $@9)
#	export SECONDO_PARAMMaxMemPerOperator=2048; \
#	$(call doTest, $@10)
#	export SECONDO_PARAMMaxMemPerOperator=2304; \
#	$(call doTest, $@11)
#	export SECONDO_PARAMMaxMemPerOperator=2560; \
#	$(call doTest, $@12)
#	export SECONDO_PARAMMaxMemPerOperator=2816; \
#	$(call doTest, $@13)
#	export SECONDO_PARAMMaxMemPerOperator=3072; \
#	$(call doTest, $@14)
#	export SECONDO_PARAMMaxMemPerOperator=3328; \
#	$(call doTest, $@15)
#	export SECONDO_PARAMMaxMemPerOperator=3584; \
#	$(call doTest, $@16)
#	export SECONDO_PARAMMaxMemPerOperator=3840; \
#	$(call doTest, $@17)
#	export SECONDO_PARAMMaxMemPerOperator=4096; \
#	$(call doTest, $@18)
#	export SECONDO_PARAMMaxMemPerOperator=4352; \
#	$(call doTest, $@19)
#	export SECONDO_PARAMMaxMemPerOperator=4608; \
#	$(call doTest, $@20)
#	export SECONDO_PARAMMaxMemPerOperator=4864; \
#	$(call doTest, $@21)
#	export SECONDO_PARAMMaxMemPerOperator=5120; \
#	$(call doTest, $@22)
#	export SECONDO_PARAMMaxMemPerOperator=5376; \
#	$(call doTest, $@23)
#	export SECONDO_PARAMMaxMemPerOperator=5632; \
#	$(call doTest, $@24)

hashjoin_progress_estimation: clean_db R50MB_T256B
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, $@1)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/$@1_proglogt.csv
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, $@2)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/$@2_proglogt.csv
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, $@3)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/$@3_proglogt.csv
	rm -f $(SECONDO_BUILD_DIR)/bin/proglogt.csv
	$(call doTest, $@4)
	cp $(SECONDO_BUILD_DIR)/bin/proglogt.csv $(LOGDIR)/$@4_proglogt.csv

dummy: clean_db \
R1MB_T64B R1MB_T128B R1MB_T256B R1MB_T512B R1MB_T1KB R1MB_T2KB R1MB_T4KB R1MB_T8KB R1MB_T16KB  R1MB_T32KB R1MB_T64KB \
R10MB_T64B R10MB_T128B R10MB_T256B R10MB_T512B R10MB_T1KB R10MB_T2KB R10MB_T4KB R10MB_T8KB R10MB_T16KB  R10MB_T32KB R10MB_T64KB \
R50MB_T64B R50MB_T128B R50MB_T256B R50MB_T512B R50MB_T1KB R50MB_T2KB R50MB_T4KB R50MB_T8KB R50MB_T16KB  R50MB_T32KB R50MB_T64KB \
R100MB_T64B R100MB_T128B R100MB_T256B R100MB_T512B R100MB_T1KB R100MB_T2KB R100MB_T4KB R100MB_T8KB R100MB_T16KB  R100MB_T32KB R100MB_T64KB 
	@echo ""
	@echo "Running experiment " $@.$(TESTEXT)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_2KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_2KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_2KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_2KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_2KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_2KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_4KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_4KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_4KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_4KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_4KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_4KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_8KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_8KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_8KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_8KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_8KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_8KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_16KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_16KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_16KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_16KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_16KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_16KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_32KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_32KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_32KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_32KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_32KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_32KB6)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_64KB1)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_64KB2)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_64KB3)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_64KB4)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_64KB5)
	export SECONDO_PARAMMaxMemPerOperator=256; \
	$(call doTest, sort_benchmark_64KB6)




