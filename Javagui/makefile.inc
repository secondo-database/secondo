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

JLEXCLASSESPATH := $(GUIPATH)/lib/JLex_v1.2.6.jar
CUPCLASSESPATH := $(GUIPATH)/lib/java_cup_v10k.jar
CUPCLASSESRUNTIME := $(GUIPATH)/lib/java_cup_v10k_runtime.jar

MP3CLASSESPATH := $(GUIPATH)/lib/jl0.4.jar

CLASSPATH := $(GUIPATH)

DOCPATH := ./doc

GUIPACKAGE := gui gui.idmanager project communication extern

