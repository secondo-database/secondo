

include ../makefile.env

.PHONY: all 
all: $(LIBDIR)/liboptparser.a

$(LIBDIR)/liboptparser.a: parser.o scanner.o  OptSecUtils.o Types.o
	$(AR) -rs  $(LIBDIR)/liboptparser.a parser.o scanner.o OptSecUtils.o Types.o

Types.o: Types.cpp Types.h
	$(CPPC) $(CCFLAGS) $(PLINCLUDEFLAGS) -c -o Types.o Types.cpp

OptSecUtils.o: OptSecUtils.cpp OptSecUtils.h
	$(CPPC) $(CCFLAGS) $(PLINCLUDEFLAGS) -c -o OptSecUtils.o OptSecUtils.cpp

parser.o: parser.c
	$(CPPC) $(CCFLAGS) $(PLINCLUDEFLAGS) -x c++ -c -o parser.o  parser.c

scanner.o: scanner.c
	$(CPPC) $(CCFLAGS) $(PLINCLUDEFLAGS) -c -o scanner.o scanner.c

scanner.c: parser.h OptParser.l Types.h
	$(LEX)  -o scanner.c OptParser.l

parser.h: parser.c

parser.c: OptParser.y OptSecUtils.h Types.h
	$(YACC)  -d -o parser.c OptParser.y

LINKFILES := $(SECONDO_BUILD_DIR)/UserInterfaces/cmsg.o $(SECONDOPL_DIR)/SecondoPL.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/libappCommon.a 

.PHONY: clean
clean:
	rm -f parser.c parser.h scanner.c scanner.o parser.o $(LIBDIR)/liboptparser.a OptChecker.o OptSecUtils.o Types.o parser.output
