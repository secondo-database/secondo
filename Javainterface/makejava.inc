#########################################################################
#									#
# SecondoInterface  A Interface for the Secondo DBMS.                   #
#									#
#########################################################################


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
#			Configurable part:				#
# 	It can be needed to modify the following lines for a correct	#
#	compilation of JavaInterface.					#
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#

# JAVAC and JAVA define the full path name of the Java compiler and java
# interpreter respectively. We recommend to use jdk 1.3 or higher. If you
# have different jdk versions installed add a prefix.

#PREFIX =
JAVAC = $(PREFIX)javac
#JAVACFLAGS = -verbose

JAVA = $(PREFIX)java


JAVAINTERFACEDIR = $(SECONDO_BUILD_DIR)/Javainterface
JAVAGUIDIR = $(SECONDO_BUILD_DIR)/Javagui

# Lexical scanner generator for java, latest version is 1.2.5
# in case of problems use Jlex 1.2.3
JLEXCLASSESPATH = $(JAVAINTERFACEDIR)/lib/JLex_v1.2.3.zip

# The following line references the file ~java_cup_v10k.zip~, which is third party
# software. Look at the lib directory for further information about it.
# This file contains the whole Cup utilities, used for generating the
# parser of the ListExpr class. We need to rebuild the .java files of the ListExpr
# parser because we have modified the NLParser.cup file.
#
# Given that the generated .java files are provided with JavaInterface, this file is not needed.
# In this case the file ~Cup_v10k_runtime.zip~, in which we ONLY
# provide the runtime classes of the Cup utilities, is needed.


CUPCLASSESPATH := $(JAVAINTERFACEDIR)/lib/java_cup_v10i.zip
CUPCLASSES_RUNTIME := $(JAVAINTERFACEDIR)/lib/java_cup_v10i_runtime.jar

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
#		End of the configurable part:				#
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


CLASSPATH := ".:$(JAVAINTERFACEDIR):$(CUPCLASSESPATH):$(JLEXCLASSESPATH):$(CUPCLASSES_RUNTIME):$(JAVAGUIDIR)"

# The next conditional ensures that a PLATFORM value exists
ifndef SECONDO_PLATFORM
$(error Variable SECONDO_PLATFORM is not defined, allowed values: win32, linux, solaris)
endif

# On Windows the classpath separator is a ";" and on Solaris/Linux it is ":".
# Also some .bat files have to be generated
ifeq ($(SECONDO_PLATFORM),win32)
 CLASSPATH := $(subst :,;, $(CLASSPATH))
 JISCRIPT_FILES := $(patsubst %.sh,%$(EXT), $(JISCRIPT_SRC))
endif
