# JAVAC and JAVA define the full path name of the Java compiler and java
# interpreter respectively. We recommend to use jdk 1.4 or higher. If you
# have different jdk versions change the names

JAVAC = javac
JAVA = java
JAVADOC = javadoc
MAKE = make

# SECONDOPATH should be a shell-variable
ifndef SECONDO_BUILD_DIR
$(error Variable SECONDO_BUILD_DIR is not defined. This variable must hold the directory of Secondo)
endif

SECONDOPATH := $(SECONDO_BUILD_DIR)

GUIPATH := $(SECONDOPATH)/Javagui

LIBPATH=$(GUIPATH)/lib
JLEX:=JLex_v1.2.6.jar
CUP:=java_cup_v10k.jar
CUPRUNTIME:=java_cup_v10k_runtime.jar
MP3:=jl0.4.jar

JLEXCLASSESPATH := $(LIBPATH)/$(JLEX)
CUPCLASSESPATH := $(LIBPATH)/$(CUP)
CUPCLASSESRUNTIME := $(LIBPATH)/$(CUPRUNTIME)
MP3CLASSESPATH := $(LIBPATH)/$(MP3)

CLASSPATH := $(GUIPATH)
JARS := $(JLEXCLASSESPATH):$(CUPCLASSESPATH):$(MP3CLASSESPATH)

DOCPATH := ./doc

