########################################################################
#
# SECONDO makefile for lists and tables
#
########################################################################

include ../../makefile.env

OBJECTS = $(INCLUDEDIR)/CTable.cpp $(INCLUDEDIR)/CTable.h

.PHONY: all 
all: $(OBJECTS)
	
$(INCLUDEDIR)/%.cpp : %.cpp
	$(CP) $< $(INCLUDEDIR)

$(INCLUDEDIR)/%.h : %.h
	$(CP) $< $(INCLUDEDIR)

.PHONY: clean
clean:
	$(RM) $(OBJECTS)
	
