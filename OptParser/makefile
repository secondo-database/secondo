

include ../makefile.env

.PHONY: all
all: $(LIBDIR)/liboptparser.a

$(LIBDIR)/liboptparser.a: parser.o scanner.o  OptSecUtils.o Types.o
	$(AR) -rs  $(LIBDIR)/liboptparser.a parser.o scanner.o OptSecUtils.o Types.o

#
# The four rules below are written out instead of using the %.o pattern rule in
# makefile.env, because each needs $(PLINCLUDEFLAGS).
Types.o: Types.cpp
	$(CPPC) $(CCFLAGS) $(PLINCLUDEFLAGS) -c -o Types.o Types.cpp \
	    -MMD -MP -MF Types.dep

OptSecUtils.o: OptSecUtils.cpp
	$(CPPC) $(CCFLAGS) $(PLINCLUDEFLAGS) -c -o OptSecUtils.o OptSecUtils.cpp \
	    -MMD -MP -MF OptSecUtils.dep

parser.o: parser.c
	$(CPPC) $(CCFLAGS) $(PLINCLUDEFLAGS) -x c++ -c -o parser.o  parser.c \
	    -MMD -MP -MF parser.dep

scanner.o: scanner.c
	$(CPPC) $(CCFLAGS) $(PLINCLUDEFLAGS) -x c++ -c -o scanner.o scanner.c \
	    -MMD -MP -MF scanner.dep

scanner.c: parser.h OptParser.l Types.h
	$(LEX)  -o scanner.c OptParser.l

parser.h: parser.c

parser.c: OptParser.y OptSecUtils.h Types.h
	$(YACC)  -d  -o parser.c OptParser.y

# makefile.env includes $(DEP_FILES), which it derives from the *.cpp here, so
# parser.c and scanner.c -- generated C -- would be left out.
-include $(wildcard *.dep)

.PHONY: clean
clean:
	rm -f parser.c parser.h scanner.c scanner.o parser.o $(LIBDIR)/liboptparser.a OptChecker.o OptSecUtils.o Types.o parser.output *.dep
