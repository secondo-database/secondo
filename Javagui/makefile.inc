# JAVAC and JAVA define the full path name of the Java compiler and java
# interpreter respectively. We recommend to use jdk 1.4 or higher. If you
# have different jdk versions change the names

JAVAC = javac
JAVA = java
JAVADOC = javadoc

# SECONDOPATH should be a shell-variable
ifndef SECONDOPATH
$(error Variable SECONDOPATH is not defined. This variable must hold the directory of Secondo)
endif

JAVAINTERFACEPATH = $(SECONDOPATH)/Javainterface
GUIPATH = $(SECONDOPATH)/Javagui

CLASSPATH = ".:$(JAVAINTERFACEPATH):$(GUIPATH)"

DOCPATH = ./doc

GUIPACKAGE = gui gui.idmanager

# the viewerpackages
# if you insert a new viewer extend this 
#
VIEWERMAINPACKAGE = viewer
HOESEVIEWERPACKAGE = viewer.hoese viewer.hoese.algebras
VIEWER3DPACKAGE = components viewer.viewer3d viewer.viewer3d.mathe \
                  viewer.viewer3d.graphic2d viewer.viewer3d.graphic3d \
                  viewer.viewer3d.objects
RELSPLITPACKAGE = viewer.relsplit
FUZZY2D = viewer.fuzzy2d  # and classes from the viewer3d

VIEWERPACKAGES = $(VIEWERMAINPACKAGE) $(HOESEVIEWERPACKAGE) $(VIEWER3DPACKAGE) \
                 $(RELSPLITPACKAGE) $(FUZZY2D)

RUNCALL="java gui.MainWindow"
RUNSCRIPTNAME = sgui

# On Windows the classpath separator is a ";" and on Solaris/Linux it is ":".
# Also some .bat files have to be generated
ifeq ($(SECONDO_PLATFORM),win32)
 CLASSPATH := $(subst :,;, $(CLASSPATH))
# JISCRIPT_FILES := $(patsubst %.sh,%$(EXT), $(JISCRIPT_SRC))
  WINSCRIPTNAME := $(RUNSCRIPTNAME).bat
endif
