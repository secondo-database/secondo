TESTRUNNER=$(SECONDO_BUILD_DIR)/bin/TestRunner
SUMMARYFILE=summary

TESTS=	refinementpartition.out \
	insideabove.out \
	trapeziumintersection.out \
	basic.out \
	atinstant.out \
	intersection.out

all: clean $(TESTS) printsummary

refinementpartition.out: refinementpartition.tests 

printsummary:
	@echo
	@echo "Test summary"
	@echo "------------"
	@echo
	@echo -n "Number of test cases: "
	@grep "^#testcase " *.tests | wc -l | sed "s/^ *//"
	@echo
	@cat $(SUMMARYFILE)
	@echo
	@echo "Please see the respective .out files to detailed test results."
	@echo

%.out: %.tests
	$(TESTRUNNER) <$< >$@ 2>&1 
	echo -n "$<: " >>$(SUMMARYFILE) 
	grep ^There $@ | grep -v warranty >>$(SUMMARYFILE)

clean:
	rm -rf *.out $(SUMMARYFILE) PagedArray* *~ cmd-counters.csv \
		secondo.log storedTypeSizes.pl tmp