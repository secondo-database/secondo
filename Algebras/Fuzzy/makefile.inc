#filename makefile
#
# last change: 05-2002 Th.Behr
#
# this is the include file for makejava

# if you want to use another compiler the change this name
JAVAC=javac

#
JAVADOC = javadoc

# set the DOCPATH
DOCPATH = ./doc

PACKAGES = fuzzyobjects fuzzyobjects.basic fuzzyobjects.simple fuzzyobjects.composite


FUZZYOBJECT_PATH = $(SECONDO_BUILD_DIR)/Algebras/Fuzzy

JAVAINTERFACE_PATH = $(SECONDO_BUILD_DIR)/Javagui/

CLASSPATH=".:$(FUZZYOBJECT_PATH):$(JAVAINTERFACE_PATH)"

ifeq ($(SECONDO_PLATFORM),win32)
 CLASSPATH := $(subst :,;, $(CLASSPATH))
# JISCRIPT_FILES := $(patsubst %.sh,%$(EXT), $(JISCRIPT_SRC))
  WINSCRIPTNAME := $(RUNSCRIPTNAME).bat
endif



