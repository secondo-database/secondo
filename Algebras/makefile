########################################################################
#
# SECONDO makefile for algebra manager and algebra modules
#
########################################################################

include ../makefile.env

.PHONY: all
all: makedirs specs

LS_COMMAND = $(addsuffix /*.spec, $(ALGEBRA_DIRS) $(JNI_ALGEBRA_DIRS) )
SPECFILES = $(shell ls $(LS_COMMAND))

specs:  $(SPECFILES)
	cat $^ | sort -b | ./specs.awk
	cat $^ | ./rm_comment_spec.awk > specs


.PHONY: makedirs
makedirs: makedirsnormal makedirsjni

# make algebras without using jni
.PHONY: makedirsnormal
makedirsnormal:
	for dir in $(ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done

# make algebras using jni
.PHONY: makedirsjni
makedirsjni:
ifeq ($(USE_JNI),"true")
	for dir in $(JNI_ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done
endif

.PHONY buildlibs:
buildlibs: buildlibsnormal buildlibsjni

.PHONY:buildlibsnormal
buildlibsnormal:
	for dir in $(ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done

.PHONY:buildlibsjni
buildlibsjni:
ifeq ($(USE_JNI),"true")
	for dir in $(JNI_ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done
endif

CLEANDIRS = $(shell find . -type d -maxdepth 1 ! -name "CVS" -and ! -name ".")
.PHONY: clean
clean:
	$(RM) specs
	for dir in $(CLEANDIRS); do \
          if !($(MAKE) -C $$dir clean) then \
            exit 1; \
          fi \
        done
