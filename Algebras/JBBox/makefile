########################################################################
#
# SECONDO makefile for JBBox Algebra
#
# 03-2004
#
########################################################################

include ../../makefile.env

# --- List of all needed header files (including indirect dependencies)

MODNAME = JBBoxAlgebra

CCFLAGS = $(DEFAULTCCFLAGS) $(JNIINCLUDE) $(LDFLAGS)

.PHONY: all
all: alljava allcpp


.PHONY: alljava
alljava:
	$(MAKE) -C bbox all

.PHONY: allcpp
allcpp: $(OBJECTS) $(LIBOBJ)

include $(DEP_FILES)

$(LIBOBJ): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) $(LDJNIFLAGS) -o $(LIBOBJ) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) $(BDBSMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBOBJ) $(OBJECTS)
endif


.PHONY: clean
clean:
	$(MAKE) -C bbox clean
	$(RM) $(LIBDIR)/$(LIBNAME).a
	$(RM) $(LIBDIR)/$(LIBNAME).so
	$(RM) $(LIBDIR)/$(LIBNAME).dll
	$(RM) $(LIBDIR)/$(LIBNAME).dll.a
	$(RM) *.o
	$(RM) *.lo
	$(RM) *.class
	$(RM) *~
	$(RM) *.tex *.aux *.toc *.pd *.dvi *.pdf *.log *.tmp


.PHONY: pdf
pdf: JBBoxAlgebra.pdf

JBBoxAlgebra.pdf: JBBoxAlgebra.pd.dvi
	dvipdfm -p a4 -o JBBoxAlgebra.pdf JBBoxAlgebra.pd.dvi


.PHONY: pd
pd: JBBoxAlgebra.pd.dvi

JBBoxAlgebra.pd.dvi: JBBoxAlgebra.cpp JBBoxAlgebra.spec bbox/Point.java bbox/BBox.java
	pd2tex JBBoxAlgebra.cpp
	grep -v '\end{document}' JBBoxAlgebra.cpp.tex >JBBoxAlgebra.cpp.tex.tmp
	cat JBBoxAlgebra.cpp.tex.tmp app1 bbox/Point.java app2 bbox/BBox.java app3 >JBBoxAlgebra.pd.tex
	latex JBBoxAlgebra.pd.tex
	latex JBBoxAlgebra.pd.tex
	latex JBBoxAlgebra.pd.tex
	rm -f JBBoxAlgebra.cpp.tex.tmp
	rm -f JBBoxAlgebra.cpp.tex
