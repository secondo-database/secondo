#This file is part of SECONDO.
#
#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.
#
#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.
#
#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# JAVAC and JAVA define the full path name of the Java compiler and java
# interpreter respectively. We recommend to use jdk 1.4 or higher. If you
# have different jdk versions change the names

ifndef SECONDO_JAVAC
JAVAC := javac
else
JAVAC := $(SECONDO_JAVAC)
endif

GCC := gcc


JAVA := java
JAVADOC := javadoc
MAKE := make

define checkjavac
	@$(JAVAC) -help > /dev/null 2>&1; \
        if [ $$? != 0 ]; then \
          echo -e "\n Error: The command \"$(JAVAC) --help\" returns an error. Check your Java SDK configuration! \n"; \
	  exit 127; \
        fi
endef

# The Java part of the tree is built serially: javac compiles the file it is
# given AND everything that file depends on, so two javac processes running at
# the same time can write the same .class file. Within one directory that is a
# real risk -- e.g. viewer/ compiles its own sources while recursing into the
# viewer subdirectories, and those pull in the same classes.
.NOTPARALLEL:

# SECONDOPATH should be a shell-variable
ifndef SECONDO_BUILD_DIR
$(error Variable SECONDO_BUILD_DIR is not defined. This variable must hold the directory of Secondo)
endif

ifndef SECONDO_PLATFORM
$(error Variable SECONDO_PLATFORM is not defined. This variable must hold the directory of Secondo)
endif

platform := $(SECONDO_PLATFORM)

SECONDOPATH := $(SECONDO_BUILD_DIR)

GUIPATH := $(SECONDOPATH)/Javagui

LIBPATH=$(GUIPATH)/lib
JLEX:=JLex_v1.2.6.jar
CUP:=java_cup_v10k.jar
CUPRUNTIME:=java_cup_v10k_runtime.jar

# switch for enabling persistent nested lists
NL_PERSISTENT=true


JLEXCLASSESPATH := $(LIBPATH)/$(JLEX)
CUPCLASSESPATH := $(LIBPATH)/$(CUP)
CUPCLASSESRUNTIME := $(LIBPATH)/$(CUPRUNTIME)

CLASSPATH := $(GUIPATH)

ifdef SECONDO_JAVART
CLASSPATH := $(CLASSPATH):$(SECONDO_JAVART)
endif

CLASSPATH := $(CLASSPATH):$(GUIPATH)/secondoInterface/SecondoInterface.jar:$(JAVA3DPATH)



JARS := $(JLEXCLASSESPATH):$(CUPCLASSESPATH)
JARS := $(JARS):$(LIBPATH)/jl1.0.1.jar
JARS := $(JARS):$(LIBPATH)/pdfbox-app-2.0.8.jar
JARS := $(JARS):$(LIBPATH)/batik-1.8.jar
JARS := $(JARS):$(LIBPATH)/grappa1_2.jar



ifdef SECONDO_SOURCE_ENCODING
ENCODING= -encoding $(SECONDO_SOURCE_ENCODING)
endif


# Historically every Java source was compiled by its own javac process, i.e.
# one JVM start per file. JVM/compiler startup dominated the build time. The
# following rule instead compiles all out-of-date sources of a directory with a
# single javac invocation. 
JAVA_STAMP := .javac-stamp

define compile-java
	$(JAVAC) $(JAVACFLAGS) -classpath $(CLASSPATH)$(if $(EXTRACP),:$(EXTRACP)) $(ENCODING) $?
	@touch $@
endef




DOCPATH := ./doc



# define standard rule for creating configuration files by
# copying the sample files.
define cp-config-file
@([ ! -f $@ ] && (cp $< $@; echo -e "Make created $@ as a copy of $<")) \
|| (echo -e "\a\n* Warning: Example file $< is newer than $@"; sleep 2)
endef

