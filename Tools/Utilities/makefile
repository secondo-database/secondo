########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../../makefile.env

OBJECTS=\
	Processes.$(OBJEXT) \
	Application.$(OBJEXT) \
	Messenger.$(OBJEXT) \
	DynamicLibrary.$(OBJEXT) \
	Profiles.$(OBJEXT) \
	FileSystem.$(OBJEXT) \
	UtilFunctions.$(OBJEXT) \
	WinUnix.$(OBJEXT)


CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(OBJECTS))

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)


.PHONY: all
all: $(CODE_DEPENDENCIES) $(OBJECTS)

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp 
	$(CC) -c -o $@ $< $(CCFLAGS)

include $(CODE_DEPENDENCIES:.cpp=.dep)


.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(CODE_DEPENDENCIES)

