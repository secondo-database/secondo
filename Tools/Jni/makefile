# 
# makefile for the jni tools
#
# last change 06-2003 Th.Behr
include ../../makefile.env
include ../../makefile.jni

.PHONY:all
all: wrapper JVMInit.o def JNITool.o

.PHONY: wrapper
wrapper:
	$(MAKE) -C wrapper all

JVMInit.o: JVMInit.cpp
	$(CC) -c $(JNIINCLUDE)  JVMInit.cpp -o JVMInit.o

JNITool.o: JNITool.cpp
	$(CC) -c $(JNIINCLUDE) $(DEFAULTCCFLAGS) JNITool.cpp -o JNITool.o


def:
ifeq ($(platform),win32)
	make -f makefile.def
endif

.PHONY:clean
clean:
	$(RM) -f *.o
	make -f makefile.def clean
	$(MAKE) -C wrapper clean
