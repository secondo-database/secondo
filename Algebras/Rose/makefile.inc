#filename makefile
#
# last change: 2003-07-03 Mirco Guenster
#
# this is the include file for makejava

# if you want to use another compiler the change this name
JAVAC=javac

#
JAVADOC = javadoc

# set the DOCPATH
DOCPATH = ./doc

PACKAGES =


FUZZYOBJECT_PATH = $(SECONDO_BUILD_DIR)/Algebras/Rose


JAVAINTERFACE_PATH = $(SECONDO_BUILD_DIR)/Javagui


CLASSPATH=".:$(JAVAINTERFACE_PATH)"

ifeq ($(SECONDO_PLATFORM),win32)
 CLASSPATH := $(subst :,;, $(CLASSPATH))
# JISCRIPT_FILES := $(patsubst %.sh,%$(EXT), $(JISCRIPT_SRC))
  WINSCRIPTNAME := $(RUNSCRIPTNAME).bat
endif



