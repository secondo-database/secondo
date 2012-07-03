


include ../makefile.env

PLLIB = -I /usr/lib/swipl-5.10.4/include

.PHONY: all 
all: $(LIBDIR)/liboptparser.a OptTest


$(LIBDIR)/liboptparser.a: parser.o scanner.o  OptSecUtils.o Types.o
	$(AR) -rs  $(LIBDIR)/liboptparser.a parser.o scanner.o OptSecUtils.o Types.o

Types.o: Types.cpp Types.h
	$(CPPC) $(CCFLAGS) $(PLLIB) -c -o Types.o Types.cpp

OptSecUtils.o: OptSecUtils.cpp OptSecUtils.h
	$(CPPC) $(CCFLAGS) $(PLLIB) -c -o OptSecUtils.o OptSecUtils.cpp


parser.o: parser.c
	$(CPPC)  $(CCFLAGS) $(PLLIB) -x c++ -c -o parser.o  parser.c

scanner.o: scanner.c
	$(CPPC) $(CCFLAGS) $(PLLIB) -c -o scanner.o scanner.c

scanner.c: parser.h OptParser.l Types.h
	$(LEX)  -o scanner.c OptParser.l

parser.h: parser.c

parser.c: OptParser.y OptSecUtils.h Types.h
	$(YACC)  -d -o parser.c OptParser.y

LINKFILES := $(SECONDO_BUILD_DIR)/UserInterfaces/cmsg.o $(SECONDOPL_DIR)/SecondoPL.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/libappCommon.a 

OptTest: OptTest.o
	$(CPPC) -o OptTest $(LINKFILES) $(PLLIB) -lsdbnl   -L/home/secondo/secondo/lib -L/usr/lib -lsdbsys -lsdbnl -lsmibdb -lsdbutils -ldb_cxx  -lpthread -lm  -L/usr/lib/swipl-5.10.4/lib/i686-linux/ -lswipl -lreadline -lgmp -loptparser -lsecondo -L/home/secondo/secondo/api_cpp/direct/lib -ljpeg -lpthread -lrecode -ldl  -ly  -lstdc++ -lgsl -lgslcblas -ljpeg  -lpthread -lpthread -lm -ldl -lrt -lstdc++ -lrecode -ldl


OptTest.o: 
	$(CPPC) -c OptTest.cpp -o OptTest.o $(CCFLAGS) $(PLINCLUDEFLAGS)

.PHONY: clean
clean:
	rm -f parser.c parser.h scanner.c scanner.o parser.o $(LIBDIR)/liboptparser.a OptChecker.o OptSecUtils.o Types.o parser.output OptTest OptTest.o
