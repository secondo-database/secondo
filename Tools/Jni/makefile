# 
# makefile for the jni tools
#
# last change 06-2003 Th.Behr
include ../../makefile.env
include ../../makefile.jni

.PHONY:all
all: JVMInit.o def

JVMInit.o: JVMInit.cpp
	$(CC) -c $(JNIINCLUDE)  JVMInit.cpp -o JVMInit.o

def:
ifeq ($(platform),win32)
	make -f makefile.def
endif

.PHONY:clean
clean:
	$(RM) -f *.o
	make -f makefile.def clean