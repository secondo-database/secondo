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
          $(MAKE) -C $$dir; \
	done

# make algebras using jni
.PHONY: makedirsjni
makedirsjni:
ifeq ($(USE_JNI),"true")
	for dir in $(JNI_ALGEBRA_DIRS); do \
          $(MAKE) -C $$dir; \
	done
endif

.PHONY buildlibs:
buildlibs: buildlibsnormal buildlibsjni

.PHONY:buildlibsnormal
buildlibsnormal:
	for dir in $(ALGEBRA_DIRS); do \
          $(MAKE) -C $$dir buildlibs; \
	done

.PHONY:buildlibsjni
buildlibsjni:
ifeq ($(USE_JNI),"true")
	for dir in $(JNI_ALGEBRA_DIRS); do \
          $(MAKE) -C $$dir; \
        done
endif


.PHONY: clean
clean:
	$(MAKE) -C Management clean
	$(RM) specs
	for dir in $(ALGEBRA_DIRS); do \
          $(MAKE) -C $$dir clean; \
	done
ifeq ($(USE_JNI),"true")
	for dir in $(JNI_ALGEBRA_DIRS); do \
          $(MAKE) -C $$dir clean; \
	done
endif
